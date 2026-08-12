#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Synchronize the documentation tree into the Sphinx source directory."""

import json
import posixpath
import re
import shutil
from html import escape
from pathlib import Path
from typing import Dict, Iterable, List
from urllib.parse import unquote, urlsplit

from .document_catalog import markdown_image_targets

from .language_support import (
    configured_language_paths,
    detect_languages,
    document_language,
    language_output_docname,
    relative_doc_url,
    repository_readme_fallbacks,
    select_default_language,
)


class FileProcessor:
    MANIFEST_NAME = ".doc_generator_manifest.json"
    DEFAULT_EXTENSIONS = [
        ".md", ".rst", ".png", ".jpg", ".jpeg", ".gif", ".svg", ".webp"
    ]

    def __init__(
        self, source_dir: str, dest_dir: str, config: Dict, catalog=None
    ):
        self.source_dir = Path(source_dir).resolve()
        self.dest_dir = Path(dest_dir).resolve()
        self.config = config
        self.catalog = catalog
        configured_extensions = config.get(
            "sync_extensions", self.DEFAULT_EXTENSIONS
        )
        self.sync_extensions = {
            extension.lower() if extension.startswith(".") else f".{extension.lower()}"
            for extension in configured_extensions
        }
        self.manifest_path = self.dest_dir / self.MANIFEST_NAME
        self.generated_paths = set()

    @staticmethod
    def _is_relative_to(path: Path, parent: Path) -> bool:
        try:
            path.relative_to(parent)
            return True
        except ValueError:
            return False

    def _safe_dest_path(self, relative_path: Path) -> Path:
        target = (self.dest_dir / relative_path).resolve()
        if not self._is_relative_to(target, self.dest_dir):
            raise ValueError(f"输出路径越界: {relative_path}")
        return target

    def cleanup_dest_dir(self):
        """Remove only files recorded by the previous synchronization."""
        self.dest_dir.mkdir(parents=True, exist_ok=True)
        if not self.manifest_path.exists():
            return

        try:
            manifest = json.loads(self.manifest_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            raise ValueError(f"无法读取生成文件清单 {self.manifest_path}: {exc}") from exc

        generated_files = manifest.get("files", [])
        parent_dirs = set()
        for relative_name in generated_files:
            target = self._safe_dest_path(Path(relative_name))
            if target.is_file() or target.is_symlink():
                target.unlink()
            parent_dirs.update(target.parents)

        for directory in sorted(parent_dirs, key=lambda item: len(item.parts), reverse=True):
            if directory == self.dest_dir or not self._is_relative_to(directory, self.dest_dir):
                continue
            try:
                directory.rmdir()
            except OSError:
                pass

        self.manifest_path.unlink(missing_ok=True)

    def sync_document_tree(self) -> List[Path]:
        """Copy configured documentation and assets while preserving paths."""
        if not self.source_dir.is_dir():
            raise FileNotFoundError(f"项目文档目录不存在: {self.source_dir}")

        copied_files = []
        if self.catalog is None:
            selected_files = (
                (source_file, source_file.relative_to(self.source_dir))
                for source_file in sorted(self.source_dir.rglob("*"))
                if source_file.is_file()
                and source_file.suffix.lower() in self.sync_extensions
            )
        else:
            selected_files = (
                (entry.source_path, entry.output_path)
                for entry in self.catalog.entries
            )

        for source_file, relative_path in selected_files:
            target = self._safe_dest_path(relative_path)
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
            self.track_generated_file(target)
            copied_files.append(relative_path)

        copied_files.extend(self._sync_repository_readme_fallbacks())
        self._rewrite_cross_language_links(copied_files)
        return copied_files

    def _rewrite_cross_language_links(self, copied_files: Iterable[Path]) -> None:
        """Keep links usable when isolated language builds exclude their targets."""
        available_languages = detect_languages(self.source_dir, self.config)
        if set(available_languages) != {"zh", "en"}:
            return

        default_language = select_default_language(
            available_languages, self.config
        )
        markdown_paths = {
            Path(relative_path)
            for relative_path in copied_files
            if Path(relative_path).suffix.lower() == ".md"
        }
        link_pattern = re.compile(
            r"(?<!!)\[[^\]]+\]\(\s*(?P<target><[^>\n]+>|[^\s)\n]+)"
        )

        for relative_path in sorted(markdown_paths):
            page = self._safe_dest_path(relative_path)
            if not page.is_file():
                continue
            content = page.read_text(encoding="utf-8")
            source_language = document_language(relative_path)
            raw_targets = {
                match.group("target").strip("<>")
                for match in link_pattern.finditer(content)
            }
            for raw_target in raw_targets:
                parsed = urlsplit(raw_target)
                if (
                    parsed.scheme
                    or parsed.netloc
                    or raw_target.startswith("#")
                    or not parsed.path.lower().endswith(".md")
                ):
                    continue
                decoded_path = unquote(parsed.path).replace("\\", "/")
                resolved = posixpath.normpath(
                    posixpath.join(relative_path.parent.as_posix(), decoded_path)
                )
                if resolved == ".." or resolved.startswith("../"):
                    continue
                target_path = Path(resolved)
                if target_path not in markdown_paths:
                    continue
                target_language = document_language(target_path)
                if target_language == source_language:
                    continue

                target_docname = language_output_docname(
                    target_path.with_suffix("").as_posix(),
                    target_language,
                    default_language,
                )
                target_url = relative_doc_url(
                    relative_path.with_suffix("").as_posix(), target_docname
                )
                content = self._replace_markdown_link_target(
                    content, parsed.path, target_url
                )
            page.write_text(content, encoding="utf-8")

    def _sync_repository_readme_fallbacks(self) -> List[Path]:
        """Copy repository README files and their referenced local images."""
        copied_files = []
        copied_asset_targets = set()
        default_pages = configured_language_paths(self.config, "default_page")
        fallback_pages = repository_readme_fallbacks(
            self.source_dir, self.config
        )
        available_languages = detect_languages(self.source_dir, self.config)
        default_language = select_default_language(
            available_languages, self.config
        )
        repository_root = self.source_dir.parent

        for language, source_file in fallback_pages.items():
            configured_target = default_pages.get(language)
            if not configured_target:
                continue
            relative_target = Path(configured_target)
            if relative_target.is_absolute() or ".." in relative_target.parts:
                raise ValueError(
                    "generation.default_page."
                    f"{language} must be relative to the documentation source: "
                    f"{configured_target}"
                )
            target = self._safe_dest_path(relative_target)
            if target.exists():
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_file, target)
            content = target.read_text(encoding="utf-8")
            for target_language, target_source in fallback_pages.items():
                if target_language == language:
                    continue
                target_page = default_pages.get(target_language)
                if not target_page:
                    continue
                source_parent = source_file.parent.relative_to(repository_root)
                source_reference = posixpath.relpath(
                    target_source.relative_to(repository_root).as_posix(),
                    start=source_parent.as_posix(),
                )
                target_docname = language_output_docname(
                    Path(target_page).with_suffix("").as_posix(),
                    target_language,
                    default_language,
                )
                target_url = relative_doc_url(
                    relative_target.with_suffix("").as_posix(),
                    target_docname,
                )
                content = self._replace_markdown_link_target(
                    content, source_reference, target_url
                )
            target.write_text(content, encoding="utf-8")
            self.track_generated_file(target)
            copied_files.append(relative_target)
            copied_files.extend(
                self._sync_repository_readme_assets(
                    source_file,
                    relative_target,
                    content,
                    copied_asset_targets,
                )
            )
        return copied_files

    def _sync_repository_readme_assets(
        self,
        source_readme: Path,
        relative_page: Path,
        content: str,
        copied_asset_targets: set,
    ) -> List[Path]:
        """Copy local images while preserving their URL relative to the fallback page."""
        repository_root = source_readme.parent.resolve()
        copied_files = []
        for raw_target in markdown_image_targets(content):
            parsed = urlsplit(raw_target.strip())
            if (
                parsed.scheme
                or parsed.netloc
                or raw_target.startswith(("#", "data:"))
            ):
                continue
            local_target = unquote(parsed.path).replace("\\", "/")
            if not local_target:
                continue
            relative_asset = Path(local_target)
            source_asset = (repository_root / relative_asset).resolve()
            if not self._is_relative_to(source_asset, repository_root):
                raise ValueError(
                    f"仓库首页 {source_readme.name} 的图片路径越界: {raw_target}"
                )
            if not source_asset.is_file():
                raise ValueError(
                    f"仓库首页 {source_readme.name} 引用了不存在的图片: "
                    f"{raw_target}"
                )

            output_relative = relative_page.parent / relative_asset
            target = self._safe_dest_path(output_relative)
            target_key = target.resolve()
            if target_key in copied_asset_targets:
                continue
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_asset, target)
            self.track_generated_file(target)
            copied_asset_targets.add(target_key)
            copied_files.append(output_relative)
        return copied_files

    @staticmethod
    def _replace_markdown_link_target(
        content: str, source_target: str, output_target: str
    ) -> str:
        aliases = {source_target}
        if not source_target.startswith((".", "/")):
            aliases.add(f"./{source_target}")
        for alias in sorted(aliases, key=len, reverse=True):
            pattern = re.compile(
                r"(?<!!)\[(?P<label>[^\]]+)\]\(\s*"
                + re.escape(alias)
                + r"(?P<url_suffix>[?#][^\s)]*)?"
                r"(?:\s+(?P<title>\"[^\"]*\"|'[^']*'))?\s*\)"
            )

            def replace_link(match):
                url = escape(
                    output_target + (match.group("url_suffix") or ""),
                    quote=True,
                )
                title = match.group("title")
                title_attribute = ""
                if title:
                    title_attribute = (
                        f' title="{escape(title[1:-1], quote=True)}"'
                    )
                return (
                    f'<a href="{url}"{title_attribute}>'
                    f'{match.group("label")}</a>'
                )

            content = pattern.sub(replace_link, content)
        return content

    def track_generated_file(self, file_path: Path):
        """Register a generated file so the next run can clean it safely."""
        resolved = Path(file_path).resolve()
        if not self._is_relative_to(resolved, self.dest_dir):
            raise ValueError(f"生成文件不在文档输出目录内: {file_path}")
        self.generated_paths.add(resolved.relative_to(self.dest_dir).as_posix())

    def write_generated_text(self, relative_path: Path, content: str):
        target = self._safe_dest_path(relative_path)
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(content, encoding="utf-8")
        self.track_generated_file(target)

    def finalize_manifest(self):
        manifest = {
            "source": str(self.source_dir),
            "files": sorted(self.generated_paths),
        }
        self.manifest_path.write_text(
            json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )

    def iter_markdown_files(self, relative_dir: Path = Path(".")) -> Iterable[Path]:
        source_dir = (self.source_dir / relative_dir).resolve()
        if not self._is_relative_to(source_dir, self.source_dir) or not source_dir.is_dir():
            return []
        return sorted(
            path.relative_to(self.source_dir)
            for path in source_dir.iterdir()
            if path.is_file() and path.suffix.lower() == ".md"
        )

    def get_readme_title(self, relative_path: Path) -> str:
        """Extract the first level-one Markdown heading."""
        source_path = self.source_dir / relative_path
        if source_path.exists():
            for line in source_path.read_text(encoding="utf-8").splitlines():
                stripped = line.strip()
                if stripped.startswith("# "):
                    return stripped[2:].strip()
        return relative_path.stem.replace("_", " ")
