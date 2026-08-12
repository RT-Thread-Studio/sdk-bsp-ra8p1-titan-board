#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Build isolated language-specific Sphinx HTML trees and merge them."""

import os
import re
import shutil
import subprocess
import sys
from html import escape
from pathlib import Path
from typing import Dict, Iterable, List, Mapping

from .language_support import (
    document_language,
    language_output_docname,
    language_root_docname,
)


SOURCE_SUFFIXES = {".md", ".rst"}
IGNORED_SOURCE_DIRECTORIES = {
    "_build",
    "source_build",
    "__pycache__",
}
PRESERVED_STATIC_FILENAMES = {"project_info.json", "project_info.js"}


def _copy_preserved_static_outputs(source_root: Path, destination_root: Path) -> None:
    """Copy final PDF metadata that must survive an HTML-only rebuild."""
    source_static = Path(source_root) / "_static"
    if not source_static.is_dir():
        return
    destination_static = Path(destination_root) / "_static"
    for source_file in source_static.iterdir():
        if not source_file.is_file() or (
            source_file.suffix.lower() != ".pdf"
            and source_file.name not in PRESERVED_STATIC_FILENAMES
        ):
            continue
        destination_static.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_file, destination_static / source_file.name)


def _source_documents(source_dir: Path) -> List[Path]:
    source_dir = Path(source_dir).resolve()
    documents = []
    for path in source_dir.rglob("*"):
        if not path.is_file() or path.suffix.lower() not in SOURCE_SUFFIXES:
            continue
        relative_path = path.relative_to(source_dir)
        if any(
            part in IGNORED_SOURCE_DIRECTORIES or part.startswith(".")
            for part in relative_path.parts[:-1]
        ):
            continue
        documents.append(relative_path)
    return sorted(documents, key=lambda item: item.as_posix().casefold())


def language_exclude_patterns(source_dir: Path, language: str) -> List[str]:
    """Enumerate opposite-language source files for one isolated build."""
    return [
        path.as_posix()
        for path in _source_documents(source_dir)
        if document_language(path) != language
    ]


def _build_one_language(
    source_dir: Path,
    output_dir: Path,
    generation: Mapping,
    language: str,
    available_languages: Iterable[str] = (),
) -> None:
    sphinx_language = "zh_CN" if language == "zh" else "en"
    master_doc = language_root_docname(source_dir, generation, language)
    build_env = os.environ.copy()
    build_env.update(
        {
            "PYTHONUTF8": "1",
            "SPHINX_LANGUAGE": sphinx_language,
            "SPHINX_MASTER_DOC": master_doc,
            "SPHINX_MASTER_DOC_OVERRIDE": master_doc,
            "SPHINX_EXCLUDE_PATTERNS": ",".join(
                language_exclude_patterns(source_dir, language)
            ),
            "DOCS_AVAILABLE_LANGUAGES": ",".join(available_languages),
        }
    )
    subprocess.run(
        [
            sys.executable,
            "-m",
            "sphinx.cmd.build",
            "-b",
            "html",
            "-D",
            f"language={sphinx_language}",
            "-D",
            f"master_doc={master_doc}",
            str(source_dir),
            str(output_dir),
        ],
        cwd=str(source_dir),
        check=True,
        env=build_env,
    )


def _rewrite_nondefault_html(
    html_root: Path, language: str, reserve_english_index: bool
) -> None:
    """Rewrite one non-default language to its isolated shared assets."""
    special_pages = {
        "search.html": f"search_{language}.html",
        "genindex.html": f"genindex_{language}.html",
    }
    if reserve_english_index:
        special_pages["index.html"] = "index_en.html"

    for html_file in html_root.rglob("*.html"):
        content = html_file.read_text(encoding="utf-8")
        rewritten = content.replace("_static/", f"_static_{language}/")
        rewritten = rewritten.replace("_images/", f"_images_{language}/")
        rewritten = rewritten.replace("_downloads/", f"_downloads_{language}/")
        for original_name, target_name in special_pages.items():
            page_pattern = re.compile(
                rf'(?P<prefix>\b(?:href|action)=["\'])'
                rf'(?P<parents>(?:\.\./)*){re.escape(original_name)}'
                rf'(?P<suffix>(?:[?#][^"\']*)?["\'])'
            )
            rewritten = page_pattern.sub(
                lambda match, replacement=target_name: (
                    f"{match.group('prefix')}{match.group('parents')}"
                    f"{replacement}{match.group('suffix')}"
                ),
                rewritten,
            )
        if html_file.name == "search.html":
            rewritten = rewritten.replace(
                'Search.loadIndex("searchindex.js")',
                f'Search.loadIndex("searchindex_{language}.js")',
            )
        if rewritten != content:
            html_file.write_text(rewritten, encoding="utf-8")


def _prepare_nondefault_language_output(
    language_dir: Path, language: str, default_language: str
) -> None:
    """Keep non-default search/static resources independent after merging."""
    reserve_english_index = language == "en" and default_language != "en"
    _rewrite_nondefault_html(
        language_dir, language, reserve_english_index
    )

    renames = {
        "_static": f"_static_{language}",
        "_images": f"_images_{language}",
        "_downloads": f"_downloads_{language}",
        "search.html": f"search_{language}.html",
        "genindex.html": f"genindex_{language}.html",
        "searchindex.js": f"searchindex_{language}.js",
    }
    if reserve_english_index:
        renames["index.html"] = "index_en.html"
    for source_name, target_name in renames.items():
        source_path = language_dir / source_name
        if source_path.exists():
            source_path.replace(language_dir / target_name)


def _prepare_english_reserved_index(
    english_dir: Path, default_language: str
) -> None:
    """Compatibility wrapper for callers of the original focused helper."""
    if default_language != "en":
        _prepare_nondefault_language_output(
            english_dir, "en", default_language
        )


def build_html_site(
    source_dir: Path,
    output_dir: Path,
    config: Mapping,
    languages: Iterable[str],
    default_language: str,
) -> Dict[str, str]:
    """Build one isolated tree per language and merge into one static site."""
    source_dir = Path(source_dir).resolve()
    output_dir = Path(output_dir).resolve()
    selected_languages = tuple(dict.fromkeys(languages))
    if not selected_languages:
        raise ValueError("没有可构建的文档语言")

    generation = config.get("generation", {}) or {}
    temporary_dirs = {
        language: output_dir.parent / f".{output_dir.name}_{language}"
        for language in selected_languages
    }
    preserved_output_dir = output_dir.parent / f".{output_dir.name}_preserved"
    shutil.rmtree(preserved_output_dir, ignore_errors=True)
    _copy_preserved_static_outputs(output_dir, preserved_output_dir)
    try:
        for language, temporary_dir in temporary_dirs.items():
            shutil.rmtree(temporary_dir, ignore_errors=True)
            print(f"  构建 {language} HTML: {temporary_dir}")
            _build_one_language(
                source_dir,
                temporary_dir,
                generation,
                language,
                selected_languages,
            )

        for language, temporary_dir in temporary_dirs.items():
            if language == default_language:
                continue
            _prepare_nondefault_language_output(
                temporary_dir, language, default_language
            )

        shutil.rmtree(output_dir, ignore_errors=True)
        output_dir.mkdir(parents=True, exist_ok=True)
        merge_order = [
            language
            for language in selected_languages
            if language != default_language
        ] + [default_language]
        for language in merge_order:
            shutil.copytree(
                temporary_dirs[language], output_dir, dirs_exist_ok=True
            )
    finally:
        _copy_preserved_static_outputs(preserved_output_dir, output_dir)
        shutil.rmtree(preserved_output_dir, ignore_errors=True)
        for temporary_dir in temporary_dirs.values():
            shutil.rmtree(temporary_dir, ignore_errors=True)

    return {
        language: language_output_docname(
            language_root_docname(source_dir, generation, language),
            language,
            default_language,
        )
        for language in selected_languages
    }


def write_site_entry(
    output_dir: Path,
    target_docname: str,
    site_title: str,
    language: str,
) -> bool:
    """Write index.html as the stable entry when the language home differs."""
    target_page = f"{target_docname}.html"
    if target_page == "index.html":
        return False

    html_language = "zh-CN" if language == "zh" else "en"
    redirect_message = (
        "正在跳转到文档首页..."
        if language == "zh"
        else "Redirecting to the documentation home page..."
    )
    redirect_link = (
        "如果页面没有自动跳转，请点击这里"
        if language == "zh"
        else "Click here if you are not redirected automatically"
    )
    safe_title = escape(str(site_title or "SDK 文档"))
    safe_target = escape(target_page, quote=True)
    entry_html = f"""<!DOCTYPE html>
<html lang="{html_language}">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{safe_title}</title>
    <meta http-equiv="refresh" content="0; url=./{safe_target}">
</head>
<body>
    <main>
        <h1>{safe_title}</h1>
        <p>{redirect_message}</p>
        <p><a href="./{safe_target}">{redirect_link}</a></p>
    </main>
</body>
</html>"""
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "index.html").write_text(entry_html, encoding="utf-8")
    return True
