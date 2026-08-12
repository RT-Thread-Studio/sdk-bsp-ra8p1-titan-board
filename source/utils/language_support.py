#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Language detection and filename conventions for generated documentation."""

import posixpath
from pathlib import Path
from typing import Dict, Iterable, Mapping, Tuple


LANGUAGE_ORDER = ("zh", "en")
DEFAULT_LANGUAGE_FILES = {
    "zh": "README_zh.md",
    "en": "README.md",
}


def language_paths(value, defaults=None) -> Dict[str, str]:
    """Normalize a language-to-path setting while supporting legacy strings."""
    fallback = defaults or DEFAULT_LANGUAGE_FILES
    if isinstance(value, Mapping):
        paths = {
            language: str(value.get(language, "") or "").replace("\\", "/")
            for language in LANGUAGE_ORDER
        }
        return {language: path for language, path in paths.items() if path}

    if isinstance(value, str) and value.strip():
        path = value.strip().replace("\\", "/")
        language = "zh" if Path(path).stem.endswith("_zh") else "en"
        return {language: path}

    return dict(fallback)


def configured_language_paths(generation: Mapping, option: str) -> Dict[str, str]:
    """Return configured paths for a root page or per-directory landing page."""
    return language_paths(generation.get(option), DEFAULT_LANGUAGE_FILES)


def language_root_docname(
    source_dir: Path, generation: Mapping, language: str
) -> str:
    """Return the existing configured home docname or its generated index."""
    configured = configured_language_paths(generation, "default_page").get(
        language, ""
    )
    relative_path = Path(configured) if configured else None
    if (
        relative_path
        and not relative_path.is_absolute()
        and ".." not in relative_path.parts
        and (Path(source_dir) / relative_path).is_file()
    ):
        return relative_path.with_suffix("").as_posix()
    return "index_zh" if language == "zh" else "index"


def language_output_docname(
    docname: str, language: str, default_language: str
) -> str:
    """Reserve root index.html for the configured non-English site entry."""
    if language == "en" and docname == "index" and default_language != "en":
        return "index_en"
    return docname


def relative_doc_url(pagename: str, target_docname: str) -> str:
    """Return a relative HTML URL without requiring a Sphinx-known target."""
    current_parent = Path(pagename).parent.as_posix()
    start = current_parent if current_parent != "." else "."
    return posixpath.relpath(f"{target_docname}.html", start=start)


def repository_readme_fallbacks(
    projects_dir: Path, generation: Mapping
) -> Dict[str, Path]:
    """Return repository-root README files when projects has no root marker."""
    detection_paths = language_paths(
        generation.get("language_detection"), DEFAULT_LANGUAGE_FILES
    )
    projects_root = Path(projects_dir).resolve()
    configured_markers = {
        language: Path(configured_path)
        for language, configured_path in detection_paths.items()
    }
    if any(
        not marker.is_absolute()
        and ".." not in marker.parts
        and (projects_root / marker).is_file()
        for marker in configured_markers.values()
    ):
        return {}

    repository_root = projects_root.parent
    return {
        language: repository_root / marker
        for language, marker in configured_markers.items()
        if not marker.is_absolute()
        and ".." not in marker.parts
        and (repository_root / marker).is_file()
    }


def detect_languages(projects_dir: Path, generation: Mapping) -> Tuple[str, ...]:
    """Detect languages from configured README markers.

    A README at the document root is authoritative.  When neither root marker
    exists, inspect directory README files so repositories that use generated
    root indexes still get the correct language set.
    """
    detection_paths = language_paths(
        generation.get("language_detection"), DEFAULT_LANGUAGE_FILES
    )
    root = Path(projects_dir)
    root_languages = tuple(
        language
        for language in LANGUAGE_ORDER
        if language in detection_paths and (root / detection_paths[language]).is_file()
    )
    if root_languages:
        return root_languages

    ignored_directories = {"_build", "source_build", "__pycache__"}

    def has_directory_marker(configured_path: str) -> bool:
        pattern = Path(configured_path).as_posix()
        for candidate in root.rglob(pattern):
            if not candidate.is_file():
                continue
            relative_parts = candidate.relative_to(root).parts[:-1]
            if any(
                part in ignored_directories or part.startswith(".")
                for part in relative_parts
            ):
                continue
            return True
        return False

    repository_languages = set(
        repository_readme_fallbacks(root, generation)
    )
    directory_languages = {
        language
        for language in LANGUAGE_ORDER
        if language in detection_paths
        and has_directory_marker(detection_paths[language])
    }
    return tuple(
        language
        for language in LANGUAGE_ORDER
        if language in repository_languages or language in directory_languages
    )


def select_default_language(
    available_languages: Iterable[str], generation: Mapping
) -> str:
    """Select the configured default, falling back to Chinese then English."""
    available = tuple(available_languages)
    configured = str(generation.get("default_language", "") or "").lower()
    if configured in available:
        return configured
    if "zh" in available:
        return "zh"
    if available:
        return available[0]
    return "zh"


def language_switch_enabled(available_languages: Iterable[str]) -> bool:
    """Show the switch only when both root language documents exist."""
    return set(available_languages) == {"zh", "en"}


def document_language(path: Path) -> str:
    """Classify a source document using the `_zh` Chinese filename suffix."""
    return "zh" if Path(path).stem.endswith("_zh") else "en"


def alternate_docname(docname: str, target_language: str) -> str:
    """Convert a Sphinx docname to the corresponding language convention."""
    path = Path(docname)
    if target_language == "zh":
        if path.name.endswith("_zh"):
            return path.as_posix()
        return path.with_name(f"{path.name}_zh").as_posix()
    if path.name.endswith("_zh"):
        return path.with_name(path.name[:-3]).as_posix()
    return path.as_posix()
