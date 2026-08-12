#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Validate the deterministic XeLaTeX environment used by PDF builds."""

from __future__ import annotations

import argparse
import os
import platform
import shutil
import subprocess
import tempfile
from urllib.request import Request, urlopen
from pathlib import Path
from typing import Mapping, Optional

import yaml


DEFAULT_PDF_FONTS = {
    "latin": "TeX Gyre Termes",
    "cjk_body": "FandolSong-Regular.otf",
    "cjk_heading": "FandolHei-Regular.otf",
    "cjk_emphasis": "FandolKai-Regular.otf",
    "code": "Source Code Pro",
}


SOURCE_CODE_PRO_MIRROR = os.environ.get(
    "SOURCE_CODE_PRO_MIRROR",
    "https://cdn.jsdelivr.net/gh/adobe-fonts/source-code-pro@release/TTF",
).rstrip("/")
SOURCE_CODE_PRO_FILES = (
    "SourceCodePro-Regular.ttf",
    "SourceCodePro-Bold.ttf",
    "SourceCodePro-It.ttf",
    "SourceCodePro-BoldIt.ttf",
)


def configured_pdf_fonts(config: Optional[Mapping] = None) -> dict[str, str]:
    """Return the exact font families configured for PDF generation."""
    generation = (config or {}).get("generation", {}) or {}
    configured = generation.get("pdf_fonts", {}) or {}
    fonts = dict(DEFAULT_PDF_FONTS)
    for key in fonts:
        value = configured.get(key)
        if value:
            fonts[key] = str(value)
    return fonts


def find_xelatex() -> Optional[str]:
    """Locate XeLaTeX on PATH or in common TeX Live/MiKTeX locations."""
    found = shutil.which("xelatex")
    if found:
        return found

    if platform.system().lower() == "windows":
        candidates = [
            r"C:\texlive\2026\bin\windows\xelatex.exe",
            r"C:\texlive\2025\bin\win32\xelatex.exe",
            r"C:\texlive\2024\bin\win32\xelatex.exe",
            r"D:\texlive\2026\bin\windows\xelatex.exe",
            r"D:\texlive\2025\bin\win32\xelatex.exe",
            r"D:\texlive\2024\bin\win32\xelatex.exe",
            r"D:\tools\texlive\2026\bin\windows\xelatex.exe",
            r"D:\tools\texlive\2025\bin\win32\xelatex.exe",
            r"C:\Program Files\MiKTeX\miktex\bin\x64\xelatex.exe",
            r"C:\Program Files (x86)\MiKTeX\miktex\bin\x64\xelatex.exe",
        ]
        for candidate in candidates:
            if Path(candidate).is_file():
                return candidate
        for root in (
            Path(r"C:\texlive"),
            Path(r"D:\texlive"),
            Path(r"D:\tools\texlive"),
        ):
            if root.is_dir():
                matches = sorted(root.glob("*/bin/*/xelatex.exe"), reverse=True)
                if matches:
                    return str(matches[0])
    return None


def _probe_source(font_name: str) -> str:
    escaped = (
        font_name.replace("\\", r"\textbackslash{}")
        .replace("{", r"\{")
        .replace("}", r"\}")
    )
    return (
        "\\documentclass{article}\n"
        "\\usepackage{fontspec}\n"
        f"\\setmainfont{{{escaped}}}\n"
        "\\begin{document}\n"
        "font probe\n"
        "\\end{document}\n"
    )


def _probe_font(xelatex: str, font_name: str) -> bool:
    with tempfile.TemporaryDirectory(prefix="sdk-docs-font-") as temp_dir:
        workdir = Path(temp_dir)
        source = workdir / "probe.tex"
        source.write_text(_probe_source(font_name), encoding="utf-8")
        result = subprocess.run(
            [
                xelatex,
                "-halt-on-error",
                "-interaction=nonstopmode",
                "-no-shell-escape",
                source.name,
            ],
            cwd=workdir,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
            timeout=45,
        )
        return result.returncode == 0


def install_pdf_system_dependencies() -> bool:
    """Best-effort installation of the same open font/tool packages used in CI."""
    system = platform.system().lower()
    try:
        if system == "linux" and shutil.which("apt-get"):
            prefix = ["sudo"] if shutil.which("sudo") else []
            subprocess.run(prefix + ["apt-get", "update"], check=False)
            packages = [
                "fontconfig",
                "fonts-dejavu-core",
                "fonts-texgyre",
                "latexmk",
                "texlive-fonts-recommended",
                "texlive-lang-chinese",
                "texlive-latex-extra",
                "texlive-latex-recommended",
                "texlive-xetex",
            ]
            result = subprocess.run(
                prefix + ["apt-get", "install", "-y", *packages],
                check=False,
            )
            if result.returncode != 0:
                return False

            font_dir = Path("/usr/local/share/fonts/source-code-pro")
            if prefix:
                subprocess.run(prefix + ["install", "-d", str(font_dir)], check=False)
            else:
                font_dir.mkdir(parents=True, exist_ok=True)
            for filename in SOURCE_CODE_PRO_FILES:
                target = font_dir / filename
                try:
                    request = Request(
                        f"{SOURCE_CODE_PRO_MIRROR}/{filename}",
                        headers={"User-Agent": "sdk-build-doc-template"},
                    )
                    with urlopen(request, timeout=45) as response:
                        data = response.read()
                    if prefix:
                        with tempfile.NamedTemporaryFile(delete=False) as temp:
                            temp.write(data)
                            temp_path = temp.name
                        install_result = subprocess.run(
                            prefix + ["install", "-m", "0644", temp_path, str(target)],
                            check=False,
                        )
                        Path(temp_path).unlink(missing_ok=True)
                        if install_result.returncode != 0:
                            print(f"[WARN] Unable to install {filename}")
                            return False
                    else:
                        target.write_bytes(data)
                except (OSError, ValueError) as exc:
                    print(f"[WARN] Unable to download {filename}: {exc}")
                    return False
            subprocess.run(prefix + ["fc-cache", "-f"], check=False)
            return True
        if system == "darwin" and shutil.which("brew"):
            subprocess.run(["brew", "install", "--quiet", "mactex-no-gui"], check=False)
            return True
        if system == "windows" and shutil.which("choco"):
            result = subprocess.run(["choco", "install", "miktex", "-y"], check=False)
            return result.returncode == 0
    except OSError as exc:
        print(f"[WARN] Unable to install PDF system dependencies automatically: {exc}")
    return False


def ensure_pdf_environment(
    config: Optional[Mapping] = None,
    *,
    auto_install: bool = True,
) -> bool:
    """Require XeLaTeX and every configured font; never use a fallback font."""
    xelatex = find_xelatex()
    fonts = configured_pdf_fonts(config)
    if not xelatex and auto_install:
        print("[INFO] XeLaTeX or PDF fonts are missing; installing system packages...")
        install_pdf_system_dependencies()
        xelatex = find_xelatex()

    if not xelatex:
        print("[ERROR] XeLaTeX is required for PDF generation but was not found.")
        return False

    missing = [
        f"{role}={family}"
        for role, family in fonts.items()
        if not _probe_font(xelatex, family)
    ]
    if missing and auto_install:
        print(
            "[INFO] Required PDF fonts are missing; "
            "retrying TeX/font package installation..."
        )
        install_pdf_system_dependencies()
        missing = [
            f"{role}={family}"
            for role, family in fonts.items()
            if not _probe_font(xelatex, family)
        ]
    if missing:
        print("[ERROR] Required PDF fonts are unavailable; refusing to use fallbacks:")
        for item in missing:
            print(f"        - {item}")
        return False

    print(f"[OK] XeLaTeX PDF environment verified ({xelatex})")
    for role, family in fonts.items():
        print(f"      {role}: {family}")
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate the PDF toolchain and fonts")
    parser.add_argument(
        "--config",
        type=Path,
        default=Path(__file__).resolve().parents[1] / "config.yaml",
        help="Documentation config file",
    )
    parser.add_argument(
        "--no-auto-install",
        action="store_true",
        help="Only validate; do not attempt system package installation",
    )
    args = parser.parse_args()
    try:
        config = yaml.safe_load(args.config.read_text(encoding="utf-8")) or {}
    except (OSError, yaml.YAMLError) as exc:
        print(f"[ERROR] Unable to read PDF configuration: {exc}")
        return 1
    return (
        0
        if ensure_pdf_environment(
            config, auto_install=not args.no_auto_install
        )
        else 1
    )


if __name__ == "__main__":
    raise SystemExit(main())
