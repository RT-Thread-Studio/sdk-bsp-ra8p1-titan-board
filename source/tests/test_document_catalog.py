import sys
import tempfile
import unittest
from pathlib import Path


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from utils.document_catalog import DocumentCatalog
from utils.file_processor import FileProcessor
from utils.index_generator import IndexGenerator


def catalog_generation():
    return {
        "discovery": {
            "mode": "project_catalog",
            "entry_files": {"zh": "README_zh.md", "en": "README.md"},
            "asset_globs": ["figures/**"],
            "unmatched_projects": "error",
            "duplicate_categories": "error",
        },
        "navigation": {"mode": "categories", "order": ["basic", "multicore"]},
        "default_page": {"zh": "README_zh.md", "en": "README.md"},
        "directory_index": {"zh": "README_zh.md", "en": "README.md"},
    }


class DocumentCatalogTests(unittest.TestCase):
    def test_project_catalog_selects_only_entry_readmes_and_figures(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            projects = Path(temp_dir) / "project"
            basic = projects / "Titan_basic_demo"
            nested = projects / "Titan_dual" / "core0"
            vendor = basic / "packages" / "vendor"
            figure = basic / "figures" / "nested"
            for directory in (basic, nested, vendor, figure):
                directory.mkdir(parents=True, exist_ok=True)
            (basic / "README_zh.md").write_text(
                "# 基础示例\n\n![图](figures/nested/demo.png)\n",
                encoding="utf-8",
            )
            (basic / "README.md").write_text("# Basic Demo\n", encoding="utf-8")
            (nested / "README_zh.md").write_text("# 核心 0\n", encoding="utf-8")
            (vendor / "README.md").write_text("# Vendor\n", encoding="utf-8")
            (figure / "demo.png").write_bytes(b"image")
            (basic / "unrelated.md").write_text("# Ignore\n", encoding="utf-8")

            categories = {
                "basic": {"patterns": ["Titan_basic_*"]},
                "multicore": {"patterns": ["Titan_dual/core0"]},
            }
            catalog = DocumentCatalog.build(
                projects, categories, catalog_generation()
            )

            self.assertEqual(
                {entry.relative_path.as_posix() for entry in catalog.entries},
                {
                    "Titan_basic_demo/README.md",
                    "Titan_basic_demo/README_zh.md",
                    "Titan_basic_demo/figures/nested/demo.png",
                    "Titan_dual/core0/README_zh.md",
                },
            )
            self.assertEqual(catalog.available_languages(), ("zh", "en"))
            self.assertEqual(
                catalog.category_projects["multicore"],
                (Path("Titan_dual/core0"),),
            )

    def test_category_navigation_uses_the_same_catalog(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repository = Path(temp_dir)
            projects = repository / "project"
            output = repository / "source"
            project = projects / "Titan_basic_demo"
            project.mkdir(parents=True)
            (project / "README_zh.md").write_text("# 点灯示例\n", encoding="utf-8")
            (repository / "README_zh.md").write_text("# 开发文档\n", encoding="utf-8")

            generation = catalog_generation()
            generation["navigation"]["order"] = ["basic"]
            categories = {"basic": {"name": "基础篇", "patterns": ["Titan_basic_*"]}}
            catalog = DocumentCatalog.build(projects, categories, generation)
            processor = FileProcessor(
                str(projects), str(output), generation, catalog=catalog
            )
            copied = processor.sync_document_tree()
            root = IndexGenerator(str(output), processor).generate_all_indexes(
                categories, {}, {"title": "开发文档"}
            )

            self.assertEqual(root, "README_zh")
            self.assertEqual(
                set(copied),
                {Path("README_zh.md"), Path("Titan_basic_demo/README_zh.md")},
            )
            category_page = output / "_navigation" / "basic_zh.rst"
            self.assertIn("基础篇", category_page.read_text(encoding="utf-8"))
            self.assertIn(
                "../Titan_basic_demo/README_zh",
                category_page.read_text(encoding="utf-8"),
            )
            self.assertIn(
                "_navigation/basic_zh",
                (output / "README_zh.md").read_text(encoding="utf-8"),
            )

    def test_project_catalog_rejects_duplicate_categories(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            projects = Path(temp_dir) / "project"
            project = projects / "Titan_basic_demo"
            project.mkdir(parents=True)
            (project / "README.md").write_text("# Demo\n", encoding="utf-8")
            generation = catalog_generation()
            generation["navigation"]["order"] = ["one", "two"]
            categories = {
                "one": {"patterns": ["Titan_basic_*"]},
                "two": {"patterns": ["Titan_basic_demo"]},
            }
            with self.assertRaisesRegex(ValueError, "同时匹配分类"):
                DocumentCatalog.build(projects, categories, generation)

    def test_project_catalog_rejects_empty_patterns_and_missing_images(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            projects = Path(temp_dir) / "project"
            projects.mkdir()
            generation = catalog_generation()
            generation["navigation"]["order"] = ["basic"]
            with self.assertRaisesRegex(ValueError, "未匹配任何目录"):
                DocumentCatalog.build(
                    projects,
                    {"basic": {"patterns": ["Titan_missing_*"]}},
                    generation,
                )

            project = projects / "Titan_basic_demo"
            project.mkdir()
            (project / "README.md").write_text(
                "# Demo\n\n![Missing](figures/missing.png)\n",
                encoding="utf-8",
            )
            with self.assertRaisesRegex(ValueError, "不存在的图片"):
                DocumentCatalog.build(
                    projects,
                    {"basic": {"patterns": ["Titan_basic_*"]}},
                    generation,
                )

    def test_unknown_navigation_category_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            projects = Path(temp_dir) / "project"
            projects.mkdir()
            generation = catalog_generation()
            generation["navigation"]["order"] = ["unknown"]
            with self.assertRaisesRegex(ValueError, "未定义分类"):
                DocumentCatalog.build(projects, {}, generation)


if __name__ == "__main__":
    unittest.main()
