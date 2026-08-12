#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Synchronize projects documentation and generate Sphinx navigation."""

import argparse
import sys
from pathlib import Path

from utils import ConfigLoader, DocumentCatalog, FileProcessor, IndexGenerator


class DocGenerator:
    def __init__(self, config_path: str = "config.yaml"):
        requested_config = Path(config_path)
        if not requested_config.exists() and requested_config == Path("config.yaml"):
            requested_config = Path(__file__).resolve().parent / "config.yaml"

        self.config_loader = ConfigLoader(str(requested_config))
        self.config_loader.validate_config()
        self.project_info = self.config_loader.get_project_info()
        self.categories = self.config_loader.get_categories()
        self.generation_config = self.config_loader.get_generation_config()

        projects_dir = self.config_loader.resolve_repository_path(
            "projects_dir", "../projects"
        )
        docs_dir = self.config_loader.resolve_repository_path("docs_dir", ".")
        self.catalog = DocumentCatalog.build(
            projects_dir, self.categories, self.generation_config
        )
        self.file_processor = FileProcessor(
            str(projects_dir),
            str(docs_dir),
            self.generation_config,
            catalog=self.catalog,
        )
        self.index_generator = IndexGenerator(str(docs_dir), self.file_processor)
        self.root_doc = "index"
        self.copied_files = []

    def run(self) -> bool:
        print("开始同步项目文档...")
        try:
            self.file_processor.cleanup_dest_dir()
            self.copied_files = self.file_processor.sync_document_tree()
            self.root_doc = self.index_generator.generate_all_indexes(
                self.categories, {}, self.project_info
            )
            self.file_processor.finalize_manifest()
            print(f"文档同步完成，共处理 {len(self.copied_files)} 个文件")
            print(f"默认页面: {self.root_doc}")
            return True
        except Exception as exc:
            if self.file_processor.generated_paths:
                self.file_processor.finalize_manifest()
            print(f"文档同步失败: {exc}")
            return False

    def get_statistics(self):
        document_count = sum(
            1 for path in self.copied_files if path.suffix.lower() in {".md", ".rst"}
        )
        return {
            "total_files": len(self.copied_files),
            "document_files": document_count,
            "asset_files": len(self.copied_files) - document_count,
            "root_doc": self.root_doc,
        }


def main():
    parser = argparse.ArgumentParser(description="SDK 文档同步与导航生成器")
    parser.add_argument(
        "--config", "-c", default="config.yaml", help="配置文件路径"
    )
    parser.add_argument("--stats", "-s", action="store_true", help="显示生成统计")
    args = parser.parse_args()

    generator = DocGenerator(args.config)
    if not generator.run():
        sys.exit(1)

    if args.stats:
        stats = generator.get_statistics()
        print(
            "统计: "
            f"文档 {stats['document_files']}，"
            f"资源 {stats['asset_files']}，"
            f"根文档 {stats['root_doc']}"
        )


if __name__ == "__main__":
    main()
