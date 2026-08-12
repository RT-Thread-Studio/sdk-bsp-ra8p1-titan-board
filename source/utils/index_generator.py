#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Generate navigation while honoring README-based directory home pages."""

import posixpath
import re
from pathlib import Path
from typing import Dict, Iterable, List

from .language_support import (
    configured_language_paths,
    detect_languages,
    document_language,
    select_default_language,
)


class IndexGenerator:
    NAVIGATION_MARKER = "<!-- generated-navigation: do not edit -->"

    def __init__(self, output_dir: str, file_processor):
        self.output_dir = Path(output_dir).resolve()
        self.file_processor = file_processor
        self.catalog = getattr(file_processor, "catalog", None)
        self.generation_config = file_processor.config
        if self.catalog is not None and self.catalog.discovery_mode == "project_catalog":
            self.available_languages = self.catalog.available_languages()
        else:
            self.available_languages = detect_languages(
                self.file_processor.source_dir, self.generation_config
            )
        self.default_language = select_default_language(
            self.available_languages, self.generation_config
        )

    @staticmethod
    def _natural_key(path: Path):
        return [
            int(part) if part.isdigit() else part.casefold()
            for part in re.split(r"(\d+)", path.as_posix())
        ]

    @staticmethod
    def _configured_path(value: str, option_name: str) -> Path:
        path = Path(value.replace("\\", "/"))
        if path.is_absolute() or ".." in path.parts:
            raise ValueError(f"{option_name} 必须是 projects 目录内的相对路径: {value}")
        return path

    @staticmethod
    def _docname(path: Path) -> str:
        return path.with_suffix("").as_posix()

    @staticmethod
    def _title_underline(title: str) -> str:
        return "=" * len(title.encode("utf-8"))

    @classmethod
    def _relative_docname(cls, path: Path, parent: Path) -> str:
        start = parent.as_posix() if parent != Path(".") else "."
        return posixpath.relpath(cls._docname(path), start=start)

    def _document_files(self) -> List[Path]:
        return sorted(
            (
                Path(relative_path)
                for relative_path in self.file_processor.generated_paths
                if Path(relative_path).suffix.lower() in {".md", ".rst"}
            ),
            key=self._natural_key,
        )

    def _language_path(self, option_name: str, language: str) -> Path:
        configured = configured_language_paths(
            self.generation_config, option_name
        ).get(language)
        if not configured:
            return Path("README_zh.md" if language == "zh" else "README.md")
        relative_index = self._configured_path(
            configured, f"generation.{option_name}.{language}"
        )
        return relative_index

    def _directory_index_path(self, directory: Path, language: str) -> Path:
        relative_index = self._language_path("directory_index", language)
        return directory / relative_index

    @staticmethod
    def _generated_index_path(directory: Path, language: str) -> Path:
        filename = "index_zh.rst" if language == "zh" else "index.rst"
        return directory / filename

    def _append_toctree(self, relative_page: Path, entries: Iterable[Path]):
        docnames = [
            self._relative_docname(entry, relative_page.parent) for entry in entries
        ]
        if not docnames:
            return

        target = self.output_dir / relative_page
        content = target.read_text(encoding="utf-8").rstrip()
        if relative_page.suffix.lower() == ".rst":
            toc_lines = [
                "",
                "",
                ".. generated-navigation",
                "",
                ".. toctree::",
                "   :maxdepth: 4",
                "   :hidden:",
                "",
                *(f"   {docname}" for docname in docnames),
                "",
            ]
        else:
            toc_lines = [
                "",
                "",
                self.NAVIGATION_MARKER,
                "```{toctree}",
                ":maxdepth: 4",
                ":hidden:",
                "",
                *docnames,
                "```",
                "",
            ]
        target.write_text(content + "\n".join(toc_lines), encoding="utf-8")
        self.file_processor.track_generated_file(target)

    def _ordered_root_directories(self, directories: Iterable[Path]) -> List[Path]:
        by_name = {directory.as_posix(): directory for directory in directories}
        configured_order = self.generation_config.get("output_structure", []) or []
        ordered = []
        for configured_name in configured_order:
            configured_path = self._configured_path(
                str(configured_name), "generation.output_structure"
            )
            if configured_path.as_posix() in by_name:
                ordered.append(by_name.pop(configured_path.as_posix()))
        ordered.extend(sorted(by_name.values(), key=self._natural_key))
        return ordered

    def _generated_section_content(
        self,
        directory: Path,
        categories: Dict,
        entries: List[Path],
        language: str,
    ) -> str:
        category = categories.get(directory.as_posix(), {})
        title_key = "name_en" if language == "en" else "name"
        description_key = "description_en" if language == "en" else "description"
        title = category.get(
            title_key,
            category.get("name", directory.name.replace("_", " ").title()),
        )
        description = category.get(
            description_key, category.get("description", "")
        )
        lines = [title, self._title_underline(title), ""]
        if description:
            lines.extend([description, ""])
        if entries:
            lines.extend(
                [
                    ".. toctree::",
                    "   :maxdepth: 4",
                    "",
                    *(f"   {self._relative_docname(entry, directory)}" for entry in entries),
                    "",
                ]
            )
        return "\n".join(lines)

    @staticmethod
    def _category_index_path(category: str, language: str) -> Path:
        suffix = "_zh" if language == "zh" else ""
        return Path("_navigation") / f"{category}{suffix}.rst"

    def _category_section_content(
        self,
        category: str,
        categories: Dict,
        entries: List[Path],
        language: str,
    ) -> str:
        node = categories.get(category, {}) or {}
        title = (
            node.get("name_en") if language == "en" else node.get("name")
        ) or node.get("name") or category.replace("_", " ").title()
        description = (
            node.get("description_en")
            if language == "en"
            else node.get("description")
        ) or node.get("description", "")
        page = self._category_index_path(category, language)
        lines = [title, self._title_underline(title), ""]
        if description:
            lines.extend([description, ""])
        if entries:
            lines.extend(
                [
                    ".. toctree::",
                    "   :maxdepth: 2",
                    "",
                    *(
                        f"   {self._relative_docname(entry, page.parent)}"
                        for entry in entries
                    ),
                    "",
                ]
            )
        return "\n".join(lines)

    def _generate_category_indexes(
        self, categories: Dict, project_info: Dict
    ) -> str:
        document_files = self._document_files()
        inferred_languages = tuple(
            language
            for language in ("zh", "en")
            if any(document_language(path) == language for path in document_files)
        )
        languages = self.available_languages or inferred_languages or (
            self.default_language,
        )
        root_pages = {}

        for language in languages:
            category_pages = []
            for category in self.catalog.categories_in_order():
                entries = [
                    entry.output_path
                    for entry in self.catalog.project_documents(language)
                    if entry.category == category
                ]
                if not entries:
                    continue
                page = self._category_index_path(category, language)
                self.file_processor.write_generated_text(
                    page,
                    self._category_section_content(
                        category, categories, entries, language
                    ),
                )
                category_pages.append(page)

            configured_default = self._language_path("default_page", language)
            default_page = (
                configured_default
                if configured_default in self._document_files()
                else None
            )
            if default_page is not None:
                self._append_toctree(default_page, category_pages)
                root_pages[language] = default_page
                continue

            title_key = "title_en" if language == "en" else "title"
            description_key = "description_en" if language == "en" else "description"
            title = project_info.get(title_key) or project_info.get(
                "title", project_info.get("name", "SDK 文档")
            )
            description = project_info.get(description_key, "")
            generated_home = self._generated_index_path(Path("."), language)
            lines = [title, self._title_underline(title), ""]
            if description:
                lines.extend([description, ""])
            if category_pages:
                lines.extend(
                    [
                        ".. toctree::",
                        "   :maxdepth: 3",
                        "",
                        *(f"   {self._docname(page)}" for page in category_pages),
                        "",
                    ]
                )
            self.file_processor.write_generated_text(
                generated_home, "\n".join(lines)
            )
            root_pages[language] = generated_home

        selected_root = root_pages.get(self.default_language)
        if selected_root is None:
            selected_root = next(iter(root_pages.values()), Path("index.rst"))
        print(
            "语言识别: "
            + ("、".join(languages) if languages else "未检测到 README")
            + f"；默认语言: {self.default_language}"
        )
        return self._docname(selected_root)

    def generate_all_indexes(
        self, categories: Dict, category_mapping: Dict, project_info: Dict
    ) -> str:
        """Build directory indexes and return the configured Sphinx root docname."""
        del category_mapping  # Kept in the signature for compatibility with older callers.
        if self.catalog is not None and self.catalog.navigation_mode == "categories":
            return self._generate_category_indexes(categories, project_info)
        document_files = self._document_files()
        inferred_languages = tuple(
            language
            for language in ("zh", "en")
            if any(document_language(path) == language for path in document_files)
        )
        languages = self.available_languages or inferred_languages or (
            self.default_language,
        )
        if not self.available_languages:
            self.default_language = select_default_language(
                languages, self.generation_config
            )
        root_pages = {}

        for language in languages:
            language_documents = [
                path
                for path in document_files
                if document_language(path) == language
            ]
            doc_directories = set()
            for document_file in language_documents:
                parent = document_file.parent
                while parent != Path("."):
                    doc_directories.add(parent)
                    parent = parent.parent

            landing_pages = {}
            for directory in sorted(
                doc_directories,
                key=lambda item: (-len(item.parts), self._natural_key(item)),
            ):
                preferred_index = self._directory_index_path(directory, language)
                generated_index = self._generated_index_path(directory, language)
                existing_candidates = (
                    (directory / "index_zh.rst", directory / "index_zh.md")
                    if language == "zh"
                    else (directory / "index.rst", directory / "index.md")
                )
                existing_index = next(
                    (
                        candidate
                        for candidate in existing_candidates
                        if candidate in language_documents
                    ),
                    None,
                )
                landing_page = (
                    preferred_index
                    if preferred_index in language_documents
                    else existing_index
                )
                direct_documents = [
                    path
                    for path in language_documents
                    if path.parent == directory and path != landing_page
                ]
                child_directories = sorted(
                    (
                        child
                        for child in doc_directories
                        if child.parent == directory and child in landing_pages
                    ),
                    key=self._natural_key,
                )
                entries = sorted(direct_documents, key=self._natural_key) + [
                    landing_pages[child] for child in child_directories
                ]

                if landing_page is not None:
                    landing_pages[directory] = landing_page
                    self._append_toctree(landing_page, entries)
                else:
                    self.file_processor.write_generated_text(
                        generated_index,
                        self._generated_section_content(
                            directory, categories, entries, language
                        ),
                    )
                    landing_pages[directory] = generated_index

            root_directories = self._ordered_root_directories(
                directory
                for directory in doc_directories
                if len(directory.parts) == 1
            )
            root_entries = [
                landing_pages[directory] for directory in root_directories
            ]
            configured_default = self._language_path("default_page", language)
            default_page = (
                configured_default
                if configured_default in language_documents
                else None
            )
            if default_page is None and self.available_languages:
                print(
                    "警告: "
                    f"{language} 默认页面不存在，改用自动首页: {configured_default}"
                )

            if default_page is None:
                default_page = next(
                    (
                        candidate
                        for candidate in (
                            self._generated_index_path(Path("."), language),
                            Path("index_zh.md") if language == "zh" else Path("index.md"),
                        )
                        if candidate in language_documents
                    ),
                    None,
                )

            root_documents = [
                path
                for path in language_documents
                if path.parent == Path(".") and path != default_page
            ]
            if default_page is not None:
                self._append_toctree(
                    default_page,
                    root_entries + sorted(root_documents, key=self._natural_key),
                )
                root_pages[language] = default_page
                continue

            title_key = "title_en" if language == "en" else "title"
            description_key = "description_en" if language == "en" else "description"
            title = project_info.get(title_key) or project_info.get(
                "title", project_info.get("name", "SDK 文档")
            )
            description = project_info.get(description_key, "")
            generated_home = self._generated_index_path(Path("."), language)
            lines = [title, self._title_underline(title), ""]
            if description:
                lines.extend([description, ""])
            fallback_entries = root_entries + sorted(
                root_documents, key=self._natural_key
            )
            if fallback_entries:
                lines.extend(
                    [
                        ".. toctree::",
                        "   :maxdepth: 4",
                        "",
                        *(f"   {self._docname(entry)}" for entry in fallback_entries),
                        "",
                    ]
                )
            self.file_processor.write_generated_text(
                generated_home, "\n".join(lines)
            )
            root_pages[language] = generated_home

        selected_root = root_pages.get(self.default_language)
        if selected_root is None:
            selected_root = next(iter(root_pages.values()), Path("index.rst"))
        print(
            "语言识别: "
            + ("、".join(languages) if languages else "未检测到 README")
            + f"；默认语言: {self.default_language}"
        )
        return self._docname(selected_root)
