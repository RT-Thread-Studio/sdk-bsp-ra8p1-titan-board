#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
本地文档构建脚本
用于快速构建和预览文档
"""

import sys
import subprocess
import argparse
import json
from pathlib import Path
from typing import List
from utils.dependency_manager import ensure_dependencies
from utils.document_catalog import DocumentCatalog
from utils.language_support import (
    detect_languages,
    select_default_language,
)
from utils.html_builder import build_html_site, write_site_entry
from utils.pdf_builder import build_detected_pdfs

SCRIPT_DIR = Path(__file__).resolve().parent
REQUIREMENTS_PATH = SCRIPT_DIR / "requirements.txt"
BUILD_ROOT = SCRIPT_DIR / "_build"
GENERATED_MANIFEST_NAME = ".doc_generator_manifest.json"


def detect_build_languages(site_config):
    """Detect languages from the same catalog used for source synchronization."""
    generation = site_config.get("generation", {}) or {}
    discovery = generation.get("discovery", {}) or {}
    discovery_mode = str(discovery.get("mode", "") or "")
    if discovery_mode != "project_catalog" and generation.get("mode") != "project_catalog":
        return detect_languages(SCRIPT_DIR, generation)

    repository = site_config.get("repository", {}) or {}
    projects_root = Path(repository.get("projects_dir", "../projects"))
    if not projects_root.is_absolute():
        projects_root = SCRIPT_DIR / projects_root
    catalog = DocumentCatalog.build(
        projects_root,
        site_config.get("categories", {}) or {},
        generation,
    )
    return catalog.available_languages()


def check_dependencies(auto_install=True):
    """检查依赖，并在需要时自动选择软件源完成安装。"""
    return ensure_dependencies(REQUIREMENTS_PATH, auto_install=auto_install)


def cleanup_temporary_build_files(build_root: Path = BUILD_ROOT) -> List[Path]:
    """Remove intermediate build files while preserving final HTML and PDF output."""
    import shutil

    build_root = Path(build_root).resolve()
    temporary_paths = (
        build_root / "latex",
        build_root / "html" / ".doctrees",
        build_root / "html" / ".buildinfo",
    )
    removed = []
    for path in temporary_paths:
        if path.is_dir():
            shutil.rmtree(path)
            removed.append(path)
        elif path.is_file() or path.is_symlink():
            path.unlink()
            removed.append(path)
    return removed


def cleanup_generated_source_files(source_root: Path = SCRIPT_DIR) -> List[Path]:
    """Remove files copied from projects after a successful documentation build."""
    source_root = Path(source_root).resolve()
    manifest_path = source_root / GENERATED_MANIFEST_NAME
    if not manifest_path.is_file():
        return []

    try:
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        print(f"[WARN] 无法读取生成文件清单，保留同步源文件: {exc}")
        return []

    generated_files = manifest.get("files", [])
    if not isinstance(generated_files, list):
        print("[WARN] 生成文件清单格式无效，保留同步源文件")
        return []

    removed = []
    parent_dirs = set()
    for relative_name in generated_files:
        if not isinstance(relative_name, str):
            continue
        relative_path = Path(relative_name)
        if relative_path.is_absolute() or ".." in relative_path.parts:
            print(f"[WARN] 忽略越界生成路径: {relative_name}")
            continue

        target = (source_root / relative_path).resolve()
        try:
            target.relative_to(source_root)
        except ValueError:
            print(f"[WARN] 忽略越界生成路径: {relative_name}")
            continue

        if target.is_file() or target.is_symlink():
            target.unlink()
            removed.append(target)
        parent_dirs.update(target.parents)

    for directory in sorted(parent_dirs, key=lambda item: len(item.parts), reverse=True):
        if directory == source_root:
            continue
        try:
            directory.relative_to(source_root)
        except ValueError:
            continue
        try:
            directory.rmdir()
            removed.append(directory)
        except OSError:
            pass

    manifest_path.unlink(missing_ok=True)
    removed.append(manifest_path)
    return removed


def build_docs(
    clean=False, serve=False, port=8000, auto_install=True, build_pdf=True
):
    """构建文档"""
    print("开始构建文档...")
    
    # 检查依赖
    if not check_dependencies(auto_install=auto_install):
        return False
    
    try:
        # 1. 生成文档结构
        print("1. 生成文档结构...")
        subprocess.run([
            sys.executable, 'doc_generator.py'
        ], cwd=str(SCRIPT_DIR), check=True)
        
        # 2. 构建HTML文档
        print("2. 构建HTML文档...")
        build_dir = SCRIPT_DIR / "_build" / "html"
        if clean and build_dir.exists():
            import shutil
            shutil.rmtree(build_dir)
            print("已清理构建目录")
        
        site_config = load_site_config()
        generation = site_config.get("generation", {}) or {}
        available_languages = detect_build_languages(site_config)
        default_language = select_default_language(
            available_languages, generation
        )
        language_roots = build_html_site(
            SCRIPT_DIR,
            build_dir,
            site_config,
            available_languages,
            default_language,
        )
        
        print(f"[OK] 文档构建完成: {build_dir.absolute()}")
        
        # 创建根目录重定向页面（本地构建时重定向到当前文档）
        create_root_redirect_local(
            build_dir,
            target_docname=language_roots[default_language],
        )

        if build_pdf:
            print("3. 生成PDF文档...")
            pdf_success, pdf_files = build_detected_pdfs(
                build_dir,
                SCRIPT_DIR,
                site_config,
                languages=available_languages,
                auto_install=auto_install,
            )
            if not pdf_success:
                return False
            for pdf_file in pdf_files:
                print(f"[OK] PDF文档: {pdf_file}")

        removed_source_paths = cleanup_generated_source_files()
        if removed_source_paths:
            print(
                f"[OK] 已清理同步源文件: "
                f"{len(removed_source_paths)} 个文件或空目录"
            )

        removed_paths = cleanup_temporary_build_files()
        if removed_paths:
            print("[OK] 已清理临时构建文件:")
            for path in removed_paths:
                print(f"      {path}")
        
        # 启动本地服务器（如果需要）
        if serve:
            step = 4 if build_pdf else 3
            print(f"{step}. 启动本地服务器 (http://localhost:{port})...")
            try:
                subprocess.run([
                    sys.executable, '-m', 'http.server', str(port)
                ], cwd=str(build_dir))
            except KeyboardInterrupt:
                print("\n服务器已停止")
        
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] 构建失败: {e}")
        return False
    except Exception as e:
        print(f"[ERROR] 未知错误: {e}")
        return False

def load_site_config():
    import yaml

    config_path = SCRIPT_DIR / "config.yaml"
    with open(config_path, "r", encoding="utf-8") as config_file:
        return yaml.safe_load(config_file) or {}


def create_root_redirect_local(build_dir, target_docname=None):
    """在根文档不是 index 时创建稳定的网站入口。"""
    config = load_site_config()
    generation = config.get("generation", {}) or {}
    project = config.get("project", {}) or {}
    available_languages = detect_build_languages(config)
    default_language = select_default_language(
        available_languages, generation
    )
    if target_docname is None:
        from utils.language_support import (
            language_output_docname,
            language_root_docname,
        )

        target_docname = language_output_docname(
            language_root_docname(
                SCRIPT_DIR, generation, default_language
            ),
            default_language,
            default_language,
        )

    site_title = project.get("title", project.get("name", "SDK 文档"))
    if not write_site_entry(
        build_dir, target_docname, site_title, default_language
    ):
        print("[OK] 自动首页已由 Sphinx 生成")
        return True

    print(f"[OK] 网站入口已指向: {target_docname}.html")
    return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="本地文档构建工具")
    parser.add_argument('--clean', action='store_true', help='清理构建目录')
    parser.add_argument('--serve', action='store_true', help='启动本地服务器')
    parser.add_argument('--port', type=int, default=8000, help='服务器端口 (默认: 8000)')
    parser.add_argument('--check', action='store_true', help='仅检查依赖')
    parser.add_argument(
        '--no-auto-install', action='store_true',
        help='缺少依赖时不自动安装'
    )
    parser.add_argument(
        '--no-pdf', action='store_true',
        help='跳过 PDF 生成，仅构建 HTML'
    )
    parser.add_argument('--check-branch', action='store_true', help='检查分支版本映射')
    parser.add_argument('--all-versions', action='store_true', help='构建所有版本（需要 --all 参数）')
    
    args = parser.parse_args()
    auto_install = not args.no_auto_install
    
    if args.check:
        sys.exit(0 if check_dependencies(auto_install=auto_install) else 1)
    
    if args.check_branch:
        if not check_dependencies(auto_install=auto_install):
            sys.exit(1)
        # 运行分支检查
        try:
            subprocess.run(
                [sys.executable, 'check_branch_versions.py'],
                cwd=str(SCRIPT_DIR),
                check=True,
            )
            return
        except subprocess.CalledProcessError:
            sys.exit(1)
    
    if args.all_versions:
        if not check_dependencies(auto_install=auto_install):
            sys.exit(1)
        # 构建所有版本
        print("构建所有版本...")
        try:
            subprocess.run(
                [sys.executable, 'version_generator.py', '--all'],
                cwd=str(SCRIPT_DIR),
                check=True,
            )
            print("\n[OK] 所有版本构建完成!")
            return
        except subprocess.CalledProcessError as e:
            print(f"\n[ERROR] 多版本构建失败: {e}")
            sys.exit(1)
    
    success = build_docs(
        clean=args.clean,
        serve=args.serve,
        port=args.port,
        auto_install=auto_install,
        build_pdf=not args.no_pdf,
    )
    
    if success:
        print("\n[OK] 构建成功!")
        if not args.serve:
            print(f"[PATH] 文档位置: {SCRIPT_DIR / '_build' / 'html'}")
            print("[TIP] 提示: 使用 --serve 参数启动本地服务器预览")
    else:
        print("\n[ERROR] 构建失败!")
        sys.exit(1)

if __name__ == "__main__":
    main()
