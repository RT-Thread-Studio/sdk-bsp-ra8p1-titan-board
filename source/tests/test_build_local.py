import sys
import tempfile
import unittest
import json
from pathlib import Path


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from build_local import (
    cleanup_generated_source_files,
    cleanup_temporary_build_files,
    write_local_version_config,
)


class BuildLocalCleanupTests(unittest.TestCase):
    def test_local_version_config_matches_repository_versions(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / ".github").mkdir()
            versions = {
                "versions": [
                    {
                        "name": "stable",
                        "display_name": "Stable",
                        "branch": "stable",
                        "url_path": "stable",
                        "description": "Stable docs",
                    },
                    {
                        "name": "next",
                        "display_name": "Next",
                        "branch": "next",
                        "url_path": "next",
                        "description": "Next docs",
                    },
                ],
                "default_version": "stable",
                "latest_version": "next",
            }
            (root / ".github" / "versions.json").write_text(
                json.dumps(versions), encoding="utf-8"
            )
            build_dir = root / "build"
            static_dir = build_dir / "_static"
            static_dir.mkdir(parents=True)
            (static_dir / "version_menu.js").write_text(
                "function getEmbeddedVersionConfig() { return null; }",
                encoding="utf-8",
            )

            write_local_version_config(build_dir, root)

            self.assertEqual(
                json.loads(
                    (static_dir / "version_config.json").read_text(
                        encoding="utf-8"
                    )
                ),
                versions,
            )
            self.assertIn(
                '"default_version": "stable"',
                (static_dir / "version_menu.js").read_text(encoding="utf-8"),
            )

    def test_cleanup_removes_generated_source_files_only(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            source_root = Path(temp_dir) / "source"
            generated_dir = source_root / "applications"
            generated_dir.mkdir(parents=True)
            (generated_dir / "README_zh.md").write_text(
                "# 应用篇\n", encoding="utf-8"
            )
            (generated_dir / "diagram.png").write_bytes(b"image")
            (generated_dir / "keep.txt").write_text(
                "手工文件", encoding="utf-8"
            )
            (source_root / "conf.py").write_text(
                "project = 'docs'\n", encoding="utf-8"
            )
            (source_root / ".doc_generator_manifest.json").write_text(
                (
                    '{"source": "projects", "files": '
                    '["applications/README_zh.md", "applications/diagram.png"]}'
                ),
                encoding="utf-8",
            )

            removed = cleanup_generated_source_files(source_root)

            self.assertFalse((generated_dir / "README_zh.md").exists())
            self.assertFalse((generated_dir / "diagram.png").exists())
            self.assertTrue((generated_dir / "keep.txt").is_file())
            self.assertTrue((source_root / "conf.py").is_file())
            self.assertFalse(
                (source_root / ".doc_generator_manifest.json").exists()
            )
            self.assertEqual(len(removed), 3)

    def test_cleanup_preserves_final_html_and_pdf(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            build_root = Path(temp_dir) / "_build"
            html_dir = build_root / "html"
            (build_root / "latex").mkdir(parents=True)
            (html_dir / ".doctrees").mkdir(parents=True)
            html_dir.mkdir(exist_ok=True)
            (html_dir / ".buildinfo").write_text("cache", encoding="utf-8")
            (html_dir / "README_zh.html").write_text("final html", encoding="utf-8")
            (html_dir / "_static").mkdir()
            (html_dir / "_static" / "Rockchip_HyperCar.pdf").write_bytes(
                b"%PDF-1.7\n"
            )

            removed = cleanup_temporary_build_files(build_root)

            self.assertEqual(
                {path.relative_to(build_root).as_posix() for path in removed},
                {"latex", "html/.doctrees", "html/.buildinfo"},
            )
            self.assertTrue((html_dir / "README_zh.html").is_file())
            self.assertTrue((html_dir / "_static" / "Rockchip_HyperCar.pdf").is_file())


if __name__ == "__main__":
    unittest.main()
