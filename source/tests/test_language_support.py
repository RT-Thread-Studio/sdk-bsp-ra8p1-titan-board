import sys
import tempfile
import unittest
from pathlib import Path


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from utils.file_processor import FileProcessor
from utils.index_generator import IndexGenerator
from utils.language_support import (
    alternate_docname,
    detect_languages,
    language_output_docname,
    language_root_docname,
    language_switch_enabled,
    relative_doc_url,
    repository_readme_fallbacks,
    select_default_language,
)


GENERATION_CONFIG = {
    "language_detection": {"zh": "README_zh.md", "en": "README.md"},
    "default_language": "zh",
    "default_page": {"zh": "README_zh.md", "en": "README.md"},
    "directory_index": {"zh": "README_zh.md", "en": "README.md"},
    "sync_extensions": [".md"],
}


class LanguageSupportTests(unittest.TestCase):
    def test_detects_chinese_english_and_bilingual_roots(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "README_zh.md").write_text("# 中文\n", encoding="utf-8")
            self.assertEqual(detect_languages(root, GENERATION_CONFIG), ("zh",))

            (root / "README_zh.md").unlink()
            (root / "README.md").write_text("# English\n", encoding="utf-8")
            self.assertEqual(detect_languages(root, GENERATION_CONFIG), ("en",))

            (root / "README_zh.md").write_text("# 中文\n", encoding="utf-8")
            self.assertEqual(
                detect_languages(root, GENERATION_CONFIG), ("zh", "en")
            )

    def test_default_language_must_be_available(self):
        self.assertEqual(select_default_language(("en",), GENERATION_CONFIG), "en")
        self.assertEqual(
            select_default_language(("zh", "en"), GENERATION_CONFIG), "zh"
        )

    def test_directory_readmes_are_used_when_root_readme_is_missing(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            guide = root / "guide"
            guide.mkdir()
            (guide / "README_zh.md").write_text("# 中文\n", encoding="utf-8")
            (guide / "README.md").write_text("# English\n", encoding="utf-8")

            self.assertEqual(
                detect_languages(root, GENERATION_CONFIG), ("zh", "en")
            )

    def test_root_readme_is_authoritative_over_directory_readmes(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            guide = root / "guide"
            guide.mkdir()
            (root / "README_zh.md").write_text("# 中文\n", encoding="utf-8")
            (guide / "README.md").write_text("# English\n", encoding="utf-8")

            self.assertEqual(detect_languages(root, GENERATION_CONFIG), ("zh",))

    def test_repository_readmes_extend_directory_language_detection(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repository = Path(temp_dir)
            projects = repository / "projects"
            guide = projects / "guide"
            guide.mkdir(parents=True)
            (repository / "README_zh.md").write_text(
                "# Repository home\n", encoding="utf-8"
            )
            (guide / "README.md").write_text(
                "# English guide\n", encoding="utf-8"
            )

            self.assertEqual(
                detect_languages(projects, GENERATION_CONFIG), ("zh", "en")
            )
            self.assertEqual(
                repository_readme_fallbacks(projects, GENERATION_CONFIG),
                {"zh": repository / "README_zh.md"},
            )

    def test_projects_root_readme_disables_repository_fallbacks(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repository = Path(temp_dir)
            projects = repository / "projects"
            projects.mkdir()
            (repository / "README.md").write_text(
                "# Repository home\n", encoding="utf-8"
            )
            (projects / "README_zh.md").write_text(
                "# Projects home\n", encoding="utf-8"
            )

            self.assertEqual(detect_languages(projects, GENERATION_CONFIG), ("zh",))
            self.assertEqual(
                repository_readme_fallbacks(projects, GENERATION_CONFIG), {}
            )

    def test_switch_requires_both_detected_languages(self):
        self.assertFalse(language_switch_enabled(()))
        self.assertFalse(language_switch_enabled(("zh",)))
        self.assertFalse(language_switch_enabled(("en",)))
        self.assertTrue(language_switch_enabled(("zh", "en")))

    def test_alternate_docname_uses_zh_suffix(self):
        self.assertEqual(alternate_docname("guide/start", "zh"), "guide/start_zh")
        self.assertEqual(alternate_docname("guide/start_zh", "en"), "guide/start")

    def test_language_home_and_output_urls_reserve_the_root_entry(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source = Path(temp_dir)
            (source / "index_zh.rst").write_text("中文\n", encoding="utf-8")
            (source / "index.rst").write_text("English\n", encoding="utf-8")

            self.assertEqual(
                language_root_docname(source, GENERATION_CONFIG, "zh"),
                "index_zh",
            )
            self.assertEqual(
                language_output_docname("index", "en", "zh"),
                "index_en",
            )
            self.assertEqual(
                relative_doc_url("guide/start_zh", "guide/start"),
                "start.html",
            )
            self.assertEqual(
                relative_doc_url("guide/start_zh", "index_en"),
                "../index_en.html",
            )

    def test_directory_readme_takes_priority_and_missing_readme_falls_back(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            output = root / "docs"
            (projects / "guide").mkdir(parents=True)
            (projects / "overview").mkdir(parents=True)
            (projects / "README_zh.md").write_text("# 首页\n", encoding="utf-8")
            (projects / "guide" / "README_zh.md").write_text(
                "# 指南\n", encoding="utf-8"
            )
            (projects / "guide" / "01_start_zh.md").write_text(
                "# 开始\n", encoding="utf-8"
            )
            (projects / "overview" / "01_intro_zh.md").write_text(
                "# 概览\n", encoding="utf-8"
            )

            processor = FileProcessor(
                str(projects), str(output), dict(GENERATION_CONFIG)
            )
            processor.sync_document_tree()
            root_doc = IndexGenerator(str(output), processor).generate_all_indexes(
                {}, {}, {"title": "测试文档"}
            )

            self.assertEqual(root_doc, "README_zh")
            self.assertIn(
                "guide/README_zh",
                (output / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "01_start_zh",
                (output / "guide" / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertTrue((output / "overview" / "index_zh.rst").is_file())

    def test_bilingual_tree_generates_separate_language_navigation(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            output = root / "docs"
            (projects / "guide").mkdir(parents=True)
            documents = {
                "README_zh.md": "# 中文首页\n",
                "README.md": "# English Home\n",
                "guide/README_zh.md": "# 中文指南\n",
                "guide/README.md": "# English Guide\n",
                "guide/start_zh.md": "# 中文开始\n",
                "guide/start.md": "# English Start\n",
            }
            for relative_name, content in documents.items():
                (projects / relative_name).write_text(content, encoding="utf-8")

            processor = FileProcessor(
                str(projects), str(output), dict(GENERATION_CONFIG)
            )
            processor.sync_document_tree()
            root_doc = IndexGenerator(str(output), processor).generate_all_indexes(
                {}, {}, {"title": "测试文档"}
            )

            self.assertEqual(root_doc, "README_zh")
            self.assertIn(
                "guide/README_zh",
                (output / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "guide/README",
                (output / "README.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "start_zh",
                (output / "guide" / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "start",
                (output / "guide" / "README.md").read_text(encoding="utf-8"),
            )

    def test_nested_bilingual_readme_links_target_the_built_html_pages(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            output = root / "source"
            guide = projects / "guide"
            guide.mkdir(parents=True)
            chinese_source = (
                "# 中文指南\n\n**中文**|[**English**](README.md)\n"
            )
            english_source = (
                "# English Guide\n\n**English**|[**Chinese**](README_zh.md)\n"
            )
            (guide / "README_zh.md").write_text(
                chinese_source, encoding="utf-8"
            )
            (guide / "README.md").write_text(
                english_source, encoding="utf-8"
            )

            processor = FileProcessor(
                str(projects), str(output), dict(GENERATION_CONFIG)
            )
            processor.sync_document_tree()

            self.assertIn(
                '**中文**|<a href="README.html">**English**</a>',
                (output / "guide" / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                '**English**|<a href="README_zh.html">**Chinese**</a>',
                (output / "guide" / "README.md").read_text(encoding="utf-8"),
            )
            self.assertEqual(
                (guide / "README_zh.md").read_text(encoding="utf-8"),
                chinese_source,
            )
            self.assertEqual(
                (guide / "README.md").read_text(encoding="utf-8"),
                english_source,
            )

    def test_repository_readmes_become_bilingual_home_pages(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repository = Path(temp_dir)
            projects = repository / "projects"
            output = repository / "source"
            guide = projects / "guide"
            guide.mkdir(parents=True)
            figures = repository / "figures"
            figures.mkdir()
            (figures / "home.png").write_bytes(b"home-image")
            (repository / "README_zh.md").write_text(
                "# Repository Chinese Home\n\n"
                "[English](./README.md)\n\nChinese introduction.\n\n"
                "![Home](figures/home.png)\n",
                encoding="utf-8",
            )
            (repository / "README.md").write_text(
                "# Repository English Home\n\n"
                "[Chinese](README_zh.md)\n\nEnglish introduction.\n\n"
                '<img src="figures/home.png" alt="Home">\n',
                encoding="utf-8",
            )
            (guide / "start_zh.md").write_text(
                "# Chinese Start\n", encoding="utf-8"
            )
            (guide / "start.md").write_text(
                "# English Start\n", encoding="utf-8"
            )

            processor = FileProcessor(
                str(projects), str(output), dict(GENERATION_CONFIG)
            )
            copied_files = processor.sync_document_tree()
            root_doc = IndexGenerator(str(output), processor).generate_all_indexes(
                {}, {}, {"title": "Test Docs"}
            )

            self.assertEqual(root_doc, "README_zh")
            self.assertIn(Path("README_zh.md"), copied_files)
            self.assertIn(Path("README.md"), copied_files)
            self.assertEqual(copied_files.count(Path("figures/home.png")), 1)
            self.assertEqual(
                (output / "figures" / "home.png").read_bytes(), b"home-image"
            )
            self.assertIn("figures/home.png", processor.generated_paths)
            self.assertIn(
                "Repository Chinese Home",
                (output / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                '<a href="README.html">English</a>',
                (output / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "guide/index_zh",
                (output / "README_zh.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "Repository English Home",
                (output / "README.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                '<a href="README_zh.html">Chinese</a>',
                (output / "README.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "guide/index",
                (output / "README.md").read_text(encoding="utf-8"),
            )
            self.assertEqual(
                (repository / "README_zh.md").read_text(encoding="utf-8"),
                "# Repository Chinese Home\n\n"
                "[English](./README.md)\n\nChinese introduction.\n\n"
                "![Home](figures/home.png)\n",
            )

    def test_repository_readme_missing_image_fails(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repository = Path(temp_dir)
            projects = repository / "projects"
            output = repository / "source"
            projects.mkdir()
            (repository / "README_zh.md").write_text(
                "# Home\n\n![Missing](figures/missing.png)\n",
                encoding="utf-8",
            )

            processor = FileProcessor(
                str(projects), str(output), dict(GENERATION_CONFIG)
            )
            with self.assertRaisesRegex(ValueError, "不存在的图片"):
                processor.sync_document_tree()


if __name__ == "__main__":
    unittest.main()
