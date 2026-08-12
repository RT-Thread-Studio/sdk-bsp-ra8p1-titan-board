#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Discover the exact document and asset set used by every build stage."""

from dataclasses import dataclass
import fnmatch
from pathlib import Path
import re
from typing import Dict, Iterable, List, Mapping, Optional, Tuple
from urllib.parse import unquote, urlsplit

from .language_support import (
    DEFAULT_LANGUAGE_FILES,
    detect_languages,
    document_language,
    language_paths,
    repository_readme_fallbacks,
)


DOCUMENT_SUFFIXES = {".md", ".rst"}


def markdown_image_targets(content: str) -> Iterable[str]:
    """Yield local or remote image targets from Markdown, MyST, RST, and HTML."""
    patterns = (
        r"!\[[^\]]*\]\(\s*(?:<(?P<angle>[^>]+)>|(?P<plain>[^\s)]+))",
        r"<img\b[^>]*\bsrc=[\"'](?P<html>[^\"']+)[\"']",
        r"^\s*\.\.\s+(?:image|figure)::\s+(?P<rst>\S+)",
        r"^\s*```\{(?:image|figure)\}\s+(?P<myst>\S+)",
    )
    for pattern in patterns:
        for match in re.finditer(
            pattern, content, flags=re.IGNORECASE | re.MULTILINE
        ):
            target = next(
                (value for value in match.groupdict().values() if value), ""
            )
            if target:
                yield target


@dataclass(frozen=True)
class DocumentEntry:
    source_path: Path
    relative_path: Path
    output_path: Path
    role: str
    language: Optional[str] = None
    project_root: Optional[Path] = None
    category: Optional[str] = None


class DocumentCatalog:
    """Validated, deterministic source selection for HTML and PDF builds."""

    SUPPORTED_DISCOVERY_MODES = {"recursive_tree", "project_catalog"}
    SUPPORTED_NAVIGATION_MODES = {"directory_tree", "categories"}

    def __init__(
        self,
        projects_root: Path,
        categories: Mapping,
        generation: Mapping,
    ):
        self.projects_root = Path(projects_root).resolve()
        self.categories = dict(categories or {})
        self.generation = dict(generation or {})
        self.discovery = dict(self.generation.get("discovery", {}) or {})
        self.navigation = dict(self.generation.get("navigation", {}) or {})
        self.discovery_mode = self._discovery_mode()
        self.navigation_mode = self._navigation_mode()
        self.navigation_order = self._navigation_order()
        self.entries: Tuple[DocumentEntry, ...] = ()
        self.category_projects: Dict[str, Tuple[Path, ...]] = {}

    @classmethod
    def build(
        cls,
        projects_root: Path,
        categories: Mapping,
        generation: Mapping,
    ) -> "DocumentCatalog":
        catalog = cls(projects_root, categories, generation)
        catalog._validate_common_config()
        if catalog.discovery_mode == "project_catalog":
            entries, category_projects = catalog._discover_project_catalog()
            catalog.category_projects = {
                category: tuple(projects)
                for category, projects in category_projects.items()
            }
        else:
            entries = catalog._discover_recursive_tree()
            catalog.category_projects = {}
        if catalog.discovery_mode == "project_catalog":
            catalog._validate_referenced_assets(entries)
        catalog.entries = tuple(
            sorted(entries, key=lambda item: item.relative_path.as_posix())
        )
        return catalog

    def _discovery_mode(self) -> str:
        configured = str(self.discovery.get("mode", "") or "").strip()
        if configured:
            return configured
        legacy = str(self.generation.get("mode", "") or "").strip()
        return "project_catalog" if legacy == "project_catalog" else "recursive_tree"

    def _navigation_mode(self) -> str:
        configured = str(self.navigation.get("mode", "") or "").strip()
        if configured:
            return configured
        return "categories" if self.discovery_mode == "project_catalog" else "directory_tree"

    def _navigation_order(self) -> Tuple[str, ...]:
        configured = self.navigation.get("order")
        if configured is None:
            configured = self.generation.get("output_structure", [])
        if not isinstance(configured, list):
            raise ValueError("generation.navigation.order 必须是分类名称列表")
        return tuple(str(item) for item in configured)

    def _validate_common_config(self) -> None:
        if not self.projects_root.is_dir():
            raise FileNotFoundError(f"项目文档目录不存在: {self.projects_root}")
        if self.discovery_mode not in self.SUPPORTED_DISCOVERY_MODES:
            supported = ", ".join(sorted(self.SUPPORTED_DISCOVERY_MODES))
            raise ValueError(
                f"不支持的 generation.discovery.mode: {self.discovery_mode}; "
                f"可选值: {supported}"
            )
        if self.navigation_mode not in self.SUPPORTED_NAVIGATION_MODES:
            supported = ", ".join(sorted(self.SUPPORTED_NAVIGATION_MODES))
            raise ValueError(
                f"不支持的 generation.navigation.mode: {self.navigation_mode}; "
                f"可选值: {supported}"
            )
        unknown = [name for name in self.navigation_order if name not in self.categories]
        if unknown:
            raise ValueError(
                "generation.navigation.order 引用了未定义分类: "
                + ", ".join(unknown)
            )
        if self.discovery_mode == "project_catalog" and self.navigation_mode != "categories":
            raise ValueError(
                "project_catalog 发现模式必须配合 generation.navigation.mode=categories"
            )

    @staticmethod
    def _safe_relative_path(value: str, option: str) -> Path:
        path = Path(str(value).replace("\\", "/"))
        if not path.parts or path.is_absolute() or ".." in path.parts:
            raise ValueError(f"{option} 必须是项目目录内的相对路径: {value}")
        return path

    def _safe_source_path(self, relative_path: Path, option: str) -> Path:
        source_path = (self.projects_root / relative_path).resolve()
        try:
            source_path.relative_to(self.projects_root)
        except ValueError as exc:
            raise ValueError(f"{option} 解析后越过 projects 根目录: {relative_path}") from exc
        return source_path

    def _sync_extensions(self) -> set:
        defaults = [
            ".md", ".rst", ".png", ".jpg", ".jpeg", ".gif", ".svg", ".webp"
        ]
        configured = self.generation.get("sync_extensions", defaults) or defaults
        return {
            value.lower() if str(value).startswith(".") else f".{str(value).lower()}"
            for value in configured
        }

    def _entry_role(self, relative_path: Path, language: Optional[str]) -> str:
        default_pages = language_paths(self.generation.get("default_page"))
        configured_home = default_pages.get(language or "")
        if configured_home and relative_path == Path(configured_home):
            return "home"

        directory_indexes = language_paths(self.generation.get("directory_index"))
        configured_index = directory_indexes.get(language or "")
        if configured_index:
            index_path = self._safe_relative_path(
                configured_index, f"generation.directory_index.{language}"
            )
            if len(relative_path.parts) >= len(index_path.parts):
                if relative_path.parts[-len(index_path.parts):] == index_path.parts:
                    return "directory_index"
        return "document"

    def _discover_recursive_tree(self) -> List[DocumentEntry]:
        extensions = self._sync_extensions()
        entries = []
        for source_path in self.projects_root.rglob("*"):
            if not source_path.is_file() or source_path.suffix.lower() not in extensions:
                continue
            relative_path = source_path.relative_to(self.projects_root)
            language = (
                document_language(relative_path)
                if source_path.suffix.lower() in DOCUMENT_SUFFIXES
                else None
            )
            category = relative_path.parts[0] if relative_path.parts else None
            entries.append(
                DocumentEntry(
                    source_path=source_path,
                    relative_path=relative_path,
                    output_path=relative_path,
                    role=(
                        self._entry_role(relative_path, language)
                        if language
                        else "asset"
                    ),
                    language=language,
                    category=category if category in self.categories else None,
                )
            )
        return entries

    def _pattern_matches(self, pattern: str) -> List[Path]:
        normalized = str(pattern).replace("\\", "/")
        self._safe_relative_path(normalized, "categories.*.patterns")
        if "/" not in normalized:
            return sorted(
                (
                    path
                    for path in self.projects_root.iterdir()
                    if path.is_dir() and fnmatch.fnmatch(path.name, normalized)
                ),
                key=lambda path: path.name.casefold(),
            )
        return sorted(
            (path for path in self.projects_root.glob(normalized) if path.is_dir()),
            key=lambda path: path.relative_to(self.projects_root).as_posix().casefold(),
        )

    def _policy_is_error(self, option: str, default: str = "error") -> bool:
        policy = str(self.discovery.get(option, default) or default).lower()
        if policy not in {"error", "ignore"}:
            raise ValueError(
                f"generation.discovery.{option} 仅支持 error 或 ignore: {policy}"
            )
        return policy == "error"

    def _discover_project_catalog(
        self,
    ) -> Tuple[List[DocumentEntry], Dict[str, List[Path]]]:
        entry_files = language_paths(
            self.discovery.get("entry_files"), DEFAULT_LANGUAGE_FILES
        )
        entry_paths = {
            language: self._safe_relative_path(
                value, f"generation.discovery.entry_files.{language}"
            )
            for language, value in entry_files.items()
        }
        asset_globs = self.discovery.get("asset_globs", ["figures/**"])
        if not isinstance(asset_globs, list):
            raise ValueError("generation.discovery.asset_globs 必须是路径模式列表")
        normalized_asset_globs = [
            self._safe_relative_path(
                value, "generation.discovery.asset_globs"
            ).as_posix()
            for value in asset_globs
        ]

        pattern_error = self._policy_is_error("unmatched_projects")
        duplicate_error = self._policy_is_error("duplicate_categories")
        category_projects: Dict[str, List[Path]] = {
            name: [] for name in self.categories
        }
        project_categories: Dict[Path, str] = {}

        for category, raw_node in self.categories.items():
            node = raw_node or {}
            patterns = node.get("patterns", []) or []
            if not isinstance(patterns, list) or not patterns:
                if pattern_error:
                    raise ValueError(f"分类 {category} 未配置 categories.{category}.patterns")
                continue
            for pattern in patterns:
                matches = self._pattern_matches(str(pattern))
                if not matches and pattern_error:
                    raise ValueError(
                        f"分类 {category} 的项目模式未匹配任何目录: {pattern}"
                    )
                for project_path in matches:
                    relative_project = project_path.relative_to(self.projects_root)
                    previous = project_categories.get(relative_project)
                    if previous and previous != category:
                        message = (
                            f"项目 {relative_project.as_posix()} 同时匹配分类 "
                            f"{previous} 和 {category}"
                        )
                        if duplicate_error:
                            raise ValueError(message)
                        continue
                    project_categories[relative_project] = category
                    if relative_project not in category_projects[category]:
                        category_projects[category].append(relative_project)

        entries: List[DocumentEntry] = []
        seen_paths = set()
        for category in self._ordered_categories(category_projects):
            for relative_project in category_projects.get(category, []):
                project_path = self._safe_source_path(relative_project, "project_catalog")
                document_count = 0
                for language, entry_path in entry_paths.items():
                    relative_path = relative_project / entry_path
                    source_path = self._safe_source_path(
                        relative_path, f"generation.discovery.entry_files.{language}"
                    )
                    if not source_path.is_file():
                        continue
                    document_count += 1
                    if relative_path not in seen_paths:
                        seen_paths.add(relative_path)
                        entries.append(
                            DocumentEntry(
                                source_path=source_path,
                                relative_path=relative_path,
                                output_path=relative_path,
                                role="project_document",
                                language=language,
                                project_root=relative_project,
                                category=category,
                            )
                        )
                if not document_count and pattern_error:
                    names = ", ".join(path.as_posix() for path in entry_paths.values())
                    raise ValueError(
                        f"项目 {relative_project.as_posix()} 不包含配置的入口文档: {names}"
                    )

                for asset_glob in normalized_asset_globs:
                    for source_path in self._asset_matches(project_path, asset_glob):
                        if not source_path.is_file():
                            continue
                        resolved = source_path.resolve()
                        try:
                            relative_path = resolved.relative_to(self.projects_root)
                        except ValueError as exc:
                            raise ValueError(
                                f"资源路径越过 projects 根目录: {source_path}"
                            ) from exc
                        if relative_path in seen_paths:
                            continue
                        seen_paths.add(relative_path)
                        entries.append(
                            DocumentEntry(
                                source_path=resolved,
                                relative_path=relative_path,
                                output_path=relative_path,
                                role="asset",
                                project_root=relative_project,
                                category=category,
                            )
                        )

        # Root README files are site home pages, not project catalog entries.
        for language, configured in language_paths(
            self.generation.get("default_page"), DEFAULT_LANGUAGE_FILES
        ).items():
            relative_path = self._safe_relative_path(
                configured, f"generation.default_page.{language}"
            )
            source_path = self._safe_source_path(
                relative_path, f"generation.default_page.{language}"
            )
            if source_path.is_file() and relative_path not in seen_paths:
                seen_paths.add(relative_path)
                entries.append(
                    DocumentEntry(
                        source_path=source_path,
                        relative_path=relative_path,
                        output_path=relative_path,
                        role="home",
                        language=language,
                    )
                )

        return entries, category_projects

    @staticmethod
    def _asset_matches(project_path: Path, asset_glob: str) -> List[Path]:
        if asset_glob.endswith("/**"):
            base = project_path / asset_glob[:-3]
            return sorted(base.rglob("*")) if base.is_dir() else []
        return sorted(project_path.glob(asset_glob))

    def _validate_referenced_assets(self, entries: List[DocumentEntry]) -> None:
        selected_paths = {entry.source_path.resolve() for entry in entries}
        for entry in entries:
            if entry.role != "project_document" or entry.source_path.suffix.lower() != ".md":
                continue
            content = entry.source_path.read_text(encoding="utf-8")
            for raw_target in markdown_image_targets(content):
                parsed = urlsplit(raw_target.strip())
                if parsed.scheme or parsed.netloc or raw_target.startswith(("#", "data:")):
                    continue
                local_target = unquote(parsed.path).replace("\\", "/")
                if not local_target:
                    continue
                candidate = (entry.source_path.parent / local_target).resolve()
                try:
                    candidate.relative_to(self.projects_root)
                except ValueError as exc:
                    raise ValueError(
                        f"入口文档 {entry.relative_path.as_posix()} 的图片路径越界: "
                        f"{raw_target}"
                    ) from exc
                if not candidate.is_file():
                    raise ValueError(
                        f"入口文档 {entry.relative_path.as_posix()} 引用了不存在的图片: "
                        f"{raw_target}"
                    )
                if candidate not in selected_paths:
                    raise ValueError(
                        f"入口文档 {entry.relative_path.as_posix()} 引用了未被 "
                        f"generation.discovery.asset_globs 收录的图片: {raw_target}"
                    )

    def _ordered_categories(
        self, mapping: Mapping[str, Iterable[Path]]
    ) -> Tuple[str, ...]:
        ordered = list(self.navigation_order)
        ordered.extend(category for category in mapping if category not in ordered)
        return tuple(ordered)

    def categories_in_order(self) -> Tuple[str, ...]:
        return self._ordered_categories(self.category_projects)

    def document_entries(
        self, language: Optional[str] = None
    ) -> Tuple[DocumentEntry, ...]:
        return tuple(
            entry
            for entry in self.entries
            if entry.role != "asset"
            and (language is None or entry.language == language)
        )

    def project_documents(
        self, language: Optional[str] = None
    ) -> Tuple[DocumentEntry, ...]:
        return tuple(
            entry
            for entry in self.entries
            if entry.role == "project_document"
            and (language is None or entry.language == language)
        )

    def languages(self) -> Tuple[str, ...]:
        present = {
            entry.language for entry in self.document_entries() if entry.language
        }
        return tuple(language for language in ("zh", "en") if language in present)

    def available_languages(self) -> Tuple[str, ...]:
        """Return languages from the same source selection used by the build."""
        if self.discovery_mode == "recursive_tree":
            return detect_languages(self.projects_root, self.generation)
        present = set(self.languages())
        present.update(repository_readme_fallbacks(self.projects_root, self.generation))
        return tuple(language for language in ("zh", "en") if language in present)
