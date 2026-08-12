#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build and validate downloadable PDFs for the detected documentation languages."""

import json
import re
from pathlib import Path
from typing import Dict, Iterable, List, Mapping, Optional, Tuple

from .language_support import detect_languages
from .pdf_environment import ensure_pdf_environment


def is_valid_pdf(path: Path) -> bool:
    """Return whether a file has a complete PDF header and cross-reference tail."""
    candidate = Path(path)
    if not candidate.is_file() or candidate.stat().st_size <= 1024:
        return False

    with candidate.open("rb") as stream:
        if stream.read(5) != b"%PDF-":
            return False
        stream.seek(max(0, candidate.stat().st_size - 8192))
        trailer = stream.read()

    return b"startxref" in trailer and trailer.rstrip().endswith(b"%%EOF")


def _safe_pdf_title(value: str) -> str:
    cleaned = re.sub(r'[<>:"/\\|?*]+', "_", str(value or "")).strip(" .")
    return cleaned or "sdk-docs"


def pdf_filename(title: str, language: str) -> str:
    safe_title = _safe_pdf_title(title)
    if language != "zh":
        safe_title = re.sub(r"\s+", "_", safe_title)
    return f"{safe_title}.pdf" if language == "zh" else f"{safe_title}_EN.pdf"


def _resolve_projects_root(docs_source: Path, config: Mapping) -> Path:
    configured = str(
        (config.get("repository", {}) or {}).get("projects_dir", "../projects")
        or "../projects"
    )
    path = Path(configured)
    if not path.is_absolute():
        path = docs_source / path
    return path.resolve()


def _write_project_info(
    static_dir: Path, project_name: str, generated_files: Dict[str, str]
) -> None:
    primary_filename = generated_files.get("zh") or generated_files.get("en", "")
    project_info = {
        "projectName": project_name,
        "pdfFileName": primary_filename,
        "pdfFiles": generated_files,
    }
    serialized = json.dumps(project_info, ensure_ascii=False)
    (static_dir / "project_info.json").write_text(serialized, encoding="utf-8")
    (static_dir / "project_info.js").write_text(
        f"window.projectInfo = {serialized};\n", encoding="utf-8"
    )


def build_detected_pdfs(
    html_dir: Path,
    docs_source: Path,
    config: Mapping,
    languages: Optional[Iterable[str]] = None,
    browser_path: Optional[str] = None,
    auto_install: bool = True,
) -> Tuple[bool, List[Path]]:
    """Generate one valid PDF per detected README language."""
    from pdf_generator_enhanced_v2 import PDFGeneratorV2

    if not ensure_pdf_environment(config, auto_install=auto_install):
        return False, []

    html_dir = Path(html_dir).resolve()
    docs_source = Path(docs_source).resolve()
    generation = config.get("generation", {}) or {}
    selected_languages = tuple(
        languages if languages is not None else detect_languages(docs_source, generation)
    )
    if not selected_languages:
        print(
            "[ERROR] 未从 projects 根目录或其文档目录的 "
            "README.md/README_zh.md 检测到可生成 PDF 的语言"
        )
        return False, []

    project = config.get("project", {}) or {}
    project_name = str(project.get("name", "SDK_Docs") or "SDK_Docs")
    safe_title = _safe_pdf_title(project_name)
    static_dir = html_dir / "_static"
    static_dir.mkdir(parents=True, exist_ok=True)
    config_path = docs_source / "config.yaml"
    generator = PDFGeneratorV2(
        html_dir,
        static_dir,
        browser_path=browser_path,
        projects_root=_resolve_projects_root(docs_source, config),
        config_path=config_path,
    )

    generated_paths = []
    generated_files = {}
    for language in selected_languages:
        expected_path = static_dir / pdf_filename(safe_title, language)
        expected_path.unlink(missing_ok=True)
        print(f"生成 {language} PDF: {expected_path.name}")
        success = generator.generate_pdf(safe_title, language=language)
        if not success or not is_valid_pdf(expected_path):
            print(f"[ERROR] PDF 未生成或文件无效: {expected_path}")
            return False, generated_paths
        generated_paths.append(expected_path)
        generated_files[language] = expected_path.name

    _write_project_info(static_dir, project_name, generated_files)
    return True, generated_paths
