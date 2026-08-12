#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Bootstrap documentation dependencies with automatic package-index selection."""

import importlib
import importlib.metadata as importlib_metadata
import os
import re
import site
import subprocess
import sys
import time
import urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple


@dataclass(frozen=True)
class PackageIndex:
    name: str
    url: str
    region: str


PACKAGE_INDEXES = (
    PackageIndex("PyPI 官方源", "https://pypi.org/simple/", "global"),
    PackageIndex("清华大学镜像", "https://pypi.tuna.tsinghua.edu.cn/simple/", "china"),
    PackageIndex("阿里云镜像", "https://mirrors.aliyun.com/pypi/simple/", "china"),
)

# (import name, distribution name)
REQUIRED_MODULES = (
    ("sphinx", "sphinx"),
    ("sphinx_rtd_theme", "sphinx_rtd_theme"),
    ("sphinx_autobuild", "sphinx-autobuild"),
    ("requests", "requests"),
    ("myst_parser", "myst-parser"),
    ("yaml", "PyYAML"),
    ("bs4", "beautifulsoup4"),
    ("markdown", "markdown"),
    ("PIL", "Pillow"),
)


def _pinned_requirements(requirements_path: Optional[Path]) -> Dict[str, str]:
    if requirements_path is None or not Path(requirements_path).is_file():
        return {}
    pins = {}
    for raw_line in Path(requirements_path).read_text(encoding="utf-8").splitlines():
        line = raw_line.split("#", 1)[0].strip()
        match = re.fullmatch(r"([A-Za-z0-9_.-]+)==([^\s;]+)(?:\s*;.*)?", line)
        if match:
            pins[match.group(1)] = match.group(2)
    return pins


def find_dependency_issues(
    requirements_path: Optional[Path] = None,
) -> Dict[str, str]:
    """Return missing or broken distributions keyed by distribution name."""
    issues = {}
    importlib.invalidate_caches()
    for module_name, distribution_name in REQUIRED_MODULES:
        try:
            importlib.import_module(module_name)
        except Exception as exc:
            issues[distribution_name] = f"{type(exc).__name__}: {exc}"
    for distribution_name, expected_version in _pinned_requirements(
        requirements_path
    ).items():
        try:
            installed_version = importlib_metadata.version(distribution_name)
        except importlib_metadata.PackageNotFoundError:
            issues[distribution_name] = "not installed"
            continue
        if installed_version != expected_version:
            issues[distribution_name] = (
                f"installed {installed_version}, required {expected_version}"
            )
    return issues


def _probe_index(index: PackageIndex, timeout: float) -> Optional[float]:
    request = urllib.request.Request(
        index.url,
        headers={"User-Agent": "sdk-docs-dependency-check/1.0"},
    )
    started_at = time.monotonic()
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response:
            response.read(1)
        return time.monotonic() - started_at
    except Exception:
        return None


def rank_package_indexes(timeout: float = 3.0) -> List[Tuple[PackageIndex, float]]:
    """Probe known indexes concurrently and return reachable ones by latency."""
    results = []
    with ThreadPoolExecutor(max_workers=len(PACKAGE_INDEXES)) as executor:
        futures = {
            executor.submit(_probe_index, index, timeout): index
            for index in PACKAGE_INDEXES
        }
        for future in as_completed(futures):
            latency = future.result()
            if latency is not None:
                results.append((futures[future], latency))
    return sorted(results, key=lambda item: item[1])


def _configured_indexes(timeout: float) -> List[PackageIndex]:
    explicit_url = (os.environ.get("DOCS_PIP_INDEX_URL", "") or "").strip()
    if explicit_url:
        print("使用 DOCS_PIP_INDEX_URL 指定的软件源")
        return [PackageIndex("用户指定源", explicit_url, "custom")]

    mirror_mode = (os.environ.get("DOCS_PIP_MIRROR", "auto") or "auto").lower()
    if mirror_mode != "auto":
        aliases = {
            "official": "global",
            "pypi": "global",
            "china": "china",
            "tsinghua": "清华大学镜像",
            "aliyun": "阿里云镜像",
        }
        requested = aliases.get(mirror_mode, mirror_mode)
        matches = [
            index
            for index in PACKAGE_INDEXES
            if requested in {index.region, index.name}
        ]
        if matches:
            return matches
        print(f"警告: 未识别 DOCS_PIP_MIRROR={mirror_mode}，改用自动探测")

    print("正在探测 Python 软件源（官方、清华、阿里云）...")
    ranked = rank_package_indexes(timeout)
    for index, latency in ranked:
        print(f"  {index.name}: {latency:.2f}s")
    pip_configured_url = (os.environ.get("PIP_INDEX_URL", "") or "").strip()
    pip_configured_index = (
        PackageIndex("pip 环境配置源", pip_configured_url, "custom")
        if pip_configured_url
        else None
    )
    if not ranked and pip_configured_index is None:
        print("  未探测到可用源，将使用 pip 当前配置")
        return []

    indexes = [index for index, _ in ranked]
    if pip_configured_index is not None:
        print("优先使用 PIP_INDEX_URL；失败后切换到探测出的备用源")
        indexes = [pip_configured_index] + [
            index for index in indexes if index.url != pip_configured_url
        ]
        return indexes

    fastest = ranked[0][0]
    if fastest.region == "china":
        print(f"检测到国内镜像链路更快，优先使用 {fastest.name}")
    else:
        print("检测到官方源链路更快，优先使用 PyPI；国内镜像作为备用")
    return indexes


def _ensure_pip() -> bool:
    check = subprocess.run(
        [sys.executable, "-m", "pip", "--version"],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    if check.returncode == 0:
        return True

    print("未检测到 pip，正在通过 ensurepip 安装...")
    bootstrap = subprocess.run(
        [sys.executable, "-m", "ensurepip", "--upgrade"], check=False
    )
    return bootstrap.returncode == 0


def _pip_install_command(requirements_path: Path, index: Optional[PackageIndex]) -> List[str]:
    command = [
        sys.executable,
        "-m",
        "pip",
        "install",
        "--disable-pip-version-check",
        "--timeout",
        os.environ.get("DOCS_PIP_TIMEOUT", "15"),
        "--retries",
        os.environ.get("DOCS_PIP_RETRIES", "2"),
        "-r",
        str(requirements_path),
    ]
    if sys.prefix == sys.base_prefix and site.ENABLE_USER_SITE:
        command.append("--user")
    if index is not None:
        command.extend(["--index-url", index.url])
    return command


def install_dependencies(requirements_path: Path, probe_timeout: float = 3.0) -> bool:
    if not requirements_path.is_file():
        print(f"错误: 找不到依赖文件 {requirements_path}")
        return False
    if not _ensure_pip():
        print("错误: pip 自动安装失败，请先安装 pip")
        return False

    indexes = _configured_indexes(probe_timeout)
    attempts: List[Optional[PackageIndex]] = indexes or [None]
    for index in attempts:
        source_name = index.name if index is not None else "pip 当前配置源"
        print(f"正在通过 {source_name} 安装文档依赖...")
        result = subprocess.run(
            _pip_install_command(requirements_path, index), check=False
        )
        if result.returncode != 0:
            print(f"通过 {source_name} 安装失败，尝试下一个可用源")
            continue

        remaining_issues = find_dependency_issues(requirements_path)
        if not remaining_issues:
            print("文档依赖安装完成")
            return True
        print("安装完成后仍有依赖无法导入: " + ", ".join(remaining_issues))

    print("错误: 所有可用软件源均安装失败")
    return False


def ensure_dependencies(
    requirements_path: Path, auto_install: bool = True, probe_timeout: float = 3.0
) -> bool:
    issues = find_dependency_issues(requirements_path)
    if not issues:
        print("文档依赖已安装")
        return True

    print("缺少或无法加载以下文档依赖:")
    for distribution_name, reason in issues.items():
        print(f"  - {distribution_name}: {reason}")
    if not auto_install:
        print(f"自动安装已禁用，请运行: {sys.executable} -m pip install -r {requirements_path}")
        return False

    return install_dependencies(requirements_path, probe_timeout)
