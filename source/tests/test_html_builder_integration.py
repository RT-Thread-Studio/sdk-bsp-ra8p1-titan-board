import shutil
import sys
import tempfile
import unittest
from pathlib import Path

import yaml


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from utils.html_builder import build_html_site, write_site_entry


class HtmlBuilderIntegrationTests(unittest.TestCase):
    def test_real_sphinx_build_keeps_language_navigation_and_assets_isolated(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            source = root / "source"
            output = root / "html"
            source.mkdir()
            shutil.copy2(SOURCE_DIR / "conf.py", source / "conf.py")
            shutil.copytree(SOURCE_DIR / "utils", source / "utils")
            shutil.copytree(SOURCE_DIR / "_static", source / "_static")
            shutil.copytree(SOURCE_DIR / "_templates", source / "_templates")

            config = {
                "project": {
                    "name": "Bilingual_Test",
                    "title": "Bilingual Test",
                    "title_en": "Bilingual Test",
                    "version": "1.0.0",
                },
                "repository": {"projects_dir": "../projects"},
                "generation": {
                    "language_detection": {
                        "zh": "README_zh.md",
                        "en": "README.md",
                    },
                    "default_language": "zh",
                    "default_page": {
                        "zh": "README_zh.md",
                        "en": "README.md",
                    },
                    "directory_index": {
                        "zh": "README_zh.md",
                        "en": "README.md",
                    },
                },
                "sphinx": {
                    "theme": "sphinx_rtd_theme",
                    "extensions": ["myst_parser"],
                    "source_suffix": {
                        ".rst": "restructuredtext",
                        ".md": "markdown",
                    },
                    "myst_extensions": ["colon_fence"],
                },
            }
            (source / "config.yaml").write_text(
                yaml.safe_dump(config, allow_unicode=True, sort_keys=False),
                encoding="utf-8",
            )
            guide = source / "guide"
            guide.mkdir()
            (source / "README_zh.md").write_text(
                """# 中文首页

```{toctree}
:hidden:

guide/README_zh
```
""",
                encoding="utf-8",
            )
            (source / "README.md").write_text(
                """# English Home

```{toctree}
:hidden:

guide/README
```
""",
                encoding="utf-8",
            )
            (guide / "README_zh.md").write_text(
                "# 中文指南\n\n中文正文。\n", encoding="utf-8"
            )
            (guide / "README.md").write_text(
                "# English Guide\n\nEnglish body.\n", encoding="utf-8"
            )

            roots = build_html_site(
                source, output, config, ("zh", "en"), "zh"
            )
            write_site_entry(output, roots["zh"], "Bilingual Test", "zh")

            chinese_page = (output / "guide" / "README_zh.html").read_text(
                encoding="utf-8"
            )
            english_page = (output / "guide" / "README.html").read_text(
                encoding="utf-8"
            )
            self.assertIn('lang="zh-CN"', chinese_page)
            self.assertIn("中文指南", chinese_page)
            self.assertNotIn("English Guide</a>", chinese_page)
            self.assertIn('lang="en"', english_page)
            self.assertIn("English Guide", english_page)
            self.assertNotIn("中文指南</a>", english_page)
            self.assertIn("../_static_en/", english_page)
            self.assertIn('targetUrl = "README_zh.html"', english_page)
            self.assertIn('targetUrl = "README.html"', chinese_page)
            self.assertTrue((output / "_static_en").is_dir())
            self.assertTrue((output / "search_en.html").is_file())
            self.assertTrue((output / "searchindex_en.js").is_file())
            language_switch = (output / "_static" / "language_switch.js").read_text(
                encoding="utf-8"
            )
            self.assertIn("getElementById('docs-language-switch')", language_switch)
            self.assertIn(".docs-language-switch__option", language_switch)
            self.assertIn("has-docs-language-switch", language_switch)
            self.assertIn("function getLanguageConfig()", language_switch)
            self.assertNotIn(
                "const config = window.DOCS_LANGUAGE", language_switch
            )
            self.assertNotIn("querySelector('.language-switch')", language_switch)
            self.assertIn(
                "url=./README_zh.html",
                (output / "index.html").read_text(encoding="utf-8"),
            )


if __name__ == "__main__":
    unittest.main()
