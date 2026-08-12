import sys
import tempfile
import unittest
from pathlib import Path


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from pdf_generator_enhanced_v2 import DocumentScanner, PDFGeneratorV2
from utils.pdf_builder import is_valid_pdf, pdf_filename
from utils.pdf_formatting import strip_manual_heading_number


class PdfBuilderTests(unittest.TestCase):
    def test_pdf_code_blocks_are_smaller_than_body_text(self):
        conf = (SOURCE_DIR / "conf.py").read_text(encoding="utf-8")

        self.assertIn("'fontpkg': ''", conf)
        self.assertIn(r"'tocdepth': r'\setcounter{tocdepth}{2}'", conf)
        self.assertIn(
            r"\fvset{fontsize=\small,baselinestretch=1.08}", conf
        )
        self.assertIn(
            r"\setmonofont{__PDF_FONT_CODE__}[Scale=MatchLowercase]", conf
        )
        self.assertIn(
            r"\protected\def\sphinxcode#1{{\rmfamily\color{inlinecodeink}#1}}",
            conf,
        )

    def test_project_catalog_pdf_ignores_nested_vendor_readmes(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "project"
            project = projects / "Titan_basic_demo"
            vendor = project / "packages" / "vendor"
            vendor.mkdir(parents=True)
            (project / "README_zh.md").write_text(
                "# 点灯示例\n\n项目正文。\n", encoding="utf-8"
            )
            (vendor / "README_zh.md").write_text(
                "# 供应商内部文档\n", encoding="utf-8"
            )
            config_path = root / "config.yaml"
            config_path.write_text(
                """categories:
  basic:
    name: "基础篇"
    patterns:
      - "Titan_basic_*"
generation:
  discovery:
    mode: "project_catalog"
    entry_files:
      zh: "README_zh.md"
      en: "README.md"
    asset_globs:
      - "figures/**"
    unmatched_projects: "error"
    duplicate_categories: "error"
  navigation:
    mode: "categories"
    order:
      - "basic"
""",
                encoding="utf-8",
            )

            scanner = DocumentScanner(root / "html", projects, config_path)
            documents = scanner.scan_documents("zh")

            self.assertEqual(list(documents), ["basic"])
            self.assertEqual(
                [doc["file"].relative_to(projects).as_posix() for doc in documents["basic"]],
                ["Titan_basic_demo/README_zh.md"],
            )
            self.assertTrue(documents["basic"][0]["standalone_directory_index"])

            generator = PDFGeneratorV2(
                root / "html",
                root / "output",
                projects_root=projects,
                config_path=config_path,
            )
            generator.project_meta = {"name": "Titan_Board"}
            _, master_path = generator._create_pdf_master_doc("zh")
            wrapper_path = root / "_pdf_directory_Titan_basic_demo_zh.rst"
            body_path = root / "Titan_basic_demo" / "_pdf_README_zh_body_zh.md"
            try:
                master_content = master_path.read_text(encoding="utf-8")
                wrapper_content = wrapper_path.read_text(encoding="utf-8")
                body_content = body_path.read_text(encoding="utf-8")
                self.assertIn(r"\pdfcategorytoc{基础篇}", master_content)
                self.assertIn("_pdf_directory_Titan_basic_demo_zh", master_content)
                self.assertIn("Titan_basic_demo/_pdf_README_zh_body_zh", wrapper_content)
                self.assertIn("项目正文。", body_content)
                self.assertNotIn("供应商内部文档", body_content)
            finally:
                master_path.unlink(missing_ok=True)
                wrapper_path.unlink(missing_ok=True)
                body_path.unlink(missing_ok=True)

    def test_scanner_reads_directory_tree_by_language(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            guide = projects / "guide"
            guide.mkdir(parents=True)
            config_path = root / "config.yaml"
            config_path.write_text(
                """categories:
  guide:
    name: "指南"
    name_en: "Guide"
generation:
  output_structure:
    - "guide"
""",
                encoding="utf-8",
            )
            documents = {
                "README_zh.md": "# 中文指南\n\n此导航正文不得进入 PDF。\n",
                "01_start_zh.md": "# 中文开始\n",
                "README.md": "# English Guide\n\nThis navigation body is not PDF content.\n",
                "01_start.md": "# English Start\n",
            }
            for name, content in documents.items():
                (guide / name).write_text(content, encoding="utf-8")
            nested = guide / "nested"
            nested.mkdir()
            (nested / "README_zh.md").write_text("# 中文子目录\n", encoding="utf-8")
            (nested / "README.md").write_text("# English Subdirectory\n", encoding="utf-8")
            (nested / "02_more_zh.md").write_text("# 中文进阶\n", encoding="utf-8")
            (nested / "02_more.md").write_text("# English Advanced\n", encoding="utf-8")

            scanner = DocumentScanner(root / "html", projects, config_path)
            chinese = scanner.scan_documents("zh")["guide"]
            english = scanner.scan_documents("en")["guide"]

            self.assertEqual(
                [doc["file"].relative_to(guide).as_posix() for doc in chinese],
                ["01_start_zh.md", "nested/02_more_zh.md"],
            )
            self.assertEqual(
                [doc["file"].relative_to(guide).as_posix() for doc in english],
                ["01_start.md", "nested/02_more.md"],
            )

            generator = PDFGeneratorV2(
                root / "html",
                root / "output",
                projects_root=projects,
                config_path=config_path,
            )
            generator.project_meta = {"name": "Demo_Docs"}
            _, master_path = generator._create_pdf_master_doc("zh")
            try:
                master_content = master_path.read_text(encoding="utf-8")
                self.assertIn("guide/01_start_zh", master_content)
                self.assertIn("_pdf_directory_guide_nested_zh", master_content)
                self.assertNotIn("README", master_content)
                self.assertNotIn(":caption:", master_content)
                self.assertIn(r"\pdfcategorytoc{中文指南}", master_content)
                self.assertNotIn("此导航正文不得进入 PDF", master_content)
                self.assertIn(":maxdepth: 2", master_content)
                wrapper = root / "_pdf_directory_guide_nested_zh.rst"
                wrapper_content = wrapper.read_text(encoding="utf-8")
                self.assertIn("中文子目录", wrapper_content)
                self.assertIn("guide/nested/02_more_zh", wrapper_content)
                wrapper.unlink(missing_ok=True)
            finally:
                master_path.unlink(missing_ok=True)

    def test_manual_heading_numbers_are_normalized_for_pdf(self):
        self.assertEqual(strip_manual_heading_number("1. 平台目标"), "平台目标")
        self.assertEqual(strip_manual_heading_number("3.1 实时性"), "实时性")
        self.assertEqual(strip_manual_heading_number("第 2 章 部署"), "部署")
        self.assertEqual(strip_manual_heading_number("20.04 LTS"), "20.04 LTS")
        self.assertEqual(strip_manual_heading_number("v1.0.0 说明"), "v1.0.0 说明")

    def test_only_configured_directory_index_is_excluded_from_body(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            guide = projects / "guide"
            guide.mkdir(parents=True)
            config_path = root / "config.yaml"
            config_path.write_text(
                """categories:
  guide:
    name: "指南"
generation:
  directory_index:
    zh: "OVERVIEW_zh.md"
    en: "OVERVIEW.md"
  output_structure:
    - "guide"
""",
                encoding="utf-8",
            )
            (guide / "OVERVIEW_zh.md").write_text(
                "# 目录标题\n\n目录导航正文。\n", encoding="utf-8"
            )
            (guide / "01_README_notes_zh.md").write_text(
                "# README 使用说明\n\n这是普通正文。\n", encoding="utf-8"
            )

            scanner = DocumentScanner(root / "html", projects, config_path)
            documents = scanner.scan_documents("zh")["guide"]

            self.assertEqual(
                [doc["file"].name for doc in documents],
                ["01_README_notes_zh.md"],
            )
            self.assertEqual(
                scanner.directory_title(guide, "zh"),
                "目录标题",
            )

    def test_scanner_appends_unconfigured_project_directories(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            example = projects / "basic_example"
            example.mkdir(parents=True)
            (example / "README_zh.md").write_text(
                "# 基础示例\n", encoding="utf-8"
            )
            (example / "01_start_zh.md").write_text(
                "# 开始\n", encoding="utf-8"
            )
            config_path = root / "config.yaml"
            config_path.write_text(
                """categories:
  basic:
    name: "基础篇"
generation:
  output_structure:
    - "basic"
""",
                encoding="utf-8",
            )

            scanner = DocumentScanner(root / "html", projects, config_path)
            documents = scanner.scan_documents("zh")

            self.assertEqual(list(documents), ["basic_example"])
            self.assertEqual(
                documents["basic_example"][0]["category_name"], "基础示例"
            )

    def test_unordered_configured_directory_keeps_its_display_name(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            guide = projects / "guide"
            guide.mkdir(parents=True)
            (guide / "01_start_zh.md").write_text(
                "# 开始\n", encoding="utf-8"
            )
            config_path = root / "config.yaml"
            config_path.write_text(
                """categories:
  guide:
    name: "开发指南"
generation:
  output_structure: []
""",
                encoding="utf-8",
            )

            scanner = DocumentScanner(root / "html", projects, config_path)
            documents = scanner.scan_documents("zh")

            self.assertEqual(
                documents["guide"][0]["category_name"], "开发指南"
            )

    def test_readme_only_directory_uses_body_without_repeating_title(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            projects = root / "projects"
            guide = projects / "guide"
            source = root / "source"
            guide.mkdir(parents=True)
            source.mkdir()
            readme = guide / "README_zh.md"
            readme.write_text(
                "# 开发指南\n\n正文说明。\n\n## 快速开始\n",
                encoding="utf-8",
            )
            config_path = source / "config.yaml"
            config_path.write_text(
                """categories:
  guide:
    name: "开发指南"
generation:
  output_structure:
    - "guide"
""",
                encoding="utf-8",
            )

            scanner = DocumentScanner(root / "html", projects, config_path)
            documents = scanner.scan_documents("zh")
            self.assertTrue(documents["guide"][0]["standalone_directory_index"])

            generator = PDFGeneratorV2(
                root / "html",
                root / "output",
                projects_root=projects,
                config_path=config_path,
            )
            generator.project_meta = {"name": "Demo_Docs"}
            _, master_path = generator._create_pdf_master_doc("zh")
            body_path = source / "guide" / "_pdf_README_zh_body_zh.md"
            try:
                master_content = master_path.read_text(encoding="utf-8")
                body_content = body_path.read_text(encoding="utf-8")
                self.assertIn("guide/_pdf_README_zh_body_zh", master_content)
                self.assertNotIn("# 开发指南", body_content)
                self.assertIn("正文说明。", body_content)
                self.assertIn("# 快速开始", body_content)
                self.assertNotIn("## 快速开始", body_content)
            finally:
                master_path.unlink(missing_ok=True)
                body_path.unlink(missing_ok=True)

    def test_pdf_validation_rejects_text_placeholders(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            placeholder = root / "placeholder.pdf"
            placeholder.write_text("PDF is still being generated", encoding="utf-8")
            self.assertFalse(is_valid_pdf(placeholder))

            truncated = root / "truncated.pdf"
            truncated.write_bytes(b"%PDF-1.4\n" + b"0" * 2048)
            self.assertFalse(is_valid_pdf(truncated))

            complete = root / "complete.pdf"
            complete.write_bytes(
                b"%PDF-1.4\n" + b"0" * 2048 + b"\nstartxref\n123\n%%EOF\n"
            )
            self.assertTrue(is_valid_pdf(complete))

    def test_language_pdf_filenames_are_stable(self):
        self.assertEqual(pdf_filename("SDK Docs", "zh"), "SDK Docs.pdf")
        self.assertEqual(pdf_filename("SDK Docs", "en"), "SDK_Docs_EN.pdf")

    def test_generator_and_validator_share_the_same_english_filename(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            output = Path(temp_dir)
            generated = []
            generator = object.__new__(PDFGeneratorV2)
            generator.output_dir = output
            generator._try_latex_pdf = (
                lambda output_pdf, language: generated.append(
                    (output_pdf, language)
                )
                or True
            )

            self.assertTrue(
                generator._generate_pdf_from_html("", "Titan-Board SDK", "en")
            )
            self.assertEqual(
                generated,
                [(output / "Titan-Board_SDK_EN.pdf", "en")],
            )


if __name__ == "__main__":
    unittest.main()
