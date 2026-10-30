import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import Mock, patch


SOURCE_DIR = Path(__file__).resolve().parents[1]
if str(SOURCE_DIR) not in sys.path:
    sys.path.insert(0, str(SOURCE_DIR))

from build_manager import BuildManager, VersionConfig
from pdf_generator_enhanced_v2 import PDFGeneratorV2


class BuildManagerWorktreeTests(unittest.TestCase):
    def setUp(self):
        self.previous_cwd = Path.cwd()
        self.temp_dir = tempfile.TemporaryDirectory()
        self.root = Path(self.temp_dir.name)
        os.chdir(self.root)
        self.git("init", "-b", "main")
        (self.root / "revision.txt").write_text("base", encoding="utf-8")
        self.git("add", "revision.txt")
        self.commit("Base documentation")
        self.git("branch", "release")
        self.git("checkout", "-b", "feature")
        (self.root / "revision.txt").write_text("pull request", encoding="utf-8")
        self.git("add", "revision.txt")
        self.commit("PR documentation")
        self.git("checkout", "--detach")
        self.manager = BuildManager.__new__(BuildManager)
        self.manager.project_root = self.root
        self.manager.docs_source = self.root / "source"
        self.manager.build_root = self.root / "output"
        self.manager.worktrees_dir = self.root / "worktrees"
        self.manager.i18n_manager = Mock()
        self.manager.i18n_manager.get_language_config.return_value = {
            "index_filename": "index.rst"
        }

    def tearDown(self):
        os.chdir(self.previous_cwd)
        self.temp_dir.cleanup()

    def git(self, *args):
        return subprocess.run(
            ["git", *args], cwd=self.root, check=True,
            capture_output=True, text=True,
        ).stdout.strip()

    def commit(self, message):
        self.git(
            "-c", "user.name=CYFS", "-c", "user.email=ci@example.invalid",
            "commit", "-m", message,
        )

    @staticmethod
    def version(branch="main"):
        return VersionConfig({
            "name": "lts", "display_name": "lts",
            "branch": branch, "url_path": "lts",
        })

    def check_revision(self, event, base_ref, branch, expected):
        with patch.dict(os.environ, {
            "GITHUB_EVENT_NAME": event, "GITHUB_BASE_REF": base_ref,
        }):
            worktree = self.manager.create_worktree(self.version(branch))
        self.assertEqual(
            (worktree / "revision.txt").read_text(encoding="utf-8"), expected
        )

    def test_pull_request_builds_checked_out_result_for_base_version(self):
        self.check_revision("pull_request", "main", "main", "pull request")

    def test_pull_request_preserves_other_version_refs(self):
        self.check_revision("pull_request", "main", "release", "base")

    def test_non_pull_request_preserves_configured_version_ref(self):
        self.check_revision("push", "", "main", "base")

    def test_pdf_uses_assets_and_config_from_the_version_worktree(self):
        worktree = self.root / "worktrees" / "lts"
        source = worktree / "source"
        source.mkdir(parents=True)
        (worktree / "project").mkdir()
        (source / "config.yaml").write_text(
            "project:\n  name: Worktree\n"
            "repository:\n  projects_dir: ../project\n", encoding="utf-8"
        )
        for filename in ("README.md", "README_zh.md"):
            (source / filename).write_text("# Docs\n", encoding="utf-8")
        image = source / "Titan_template" / "figures" / "big.png"
        image.parent.mkdir(parents=True)
        image.write_bytes(b"worktree-only asset")

        class ReachedPdfBuild(Exception):
            pass

        def inspect_pdf(generator, title, language):
            self.assertEqual(generator.config_path.parent, source)
            self.assertEqual(generator.scanner.projects_root, worktree / "project")
            self.assertEqual(
                (generator.config_path.parent / "Titan_template/figures/big.png")
                .read_bytes(), b"worktree-only asset",
            )
            raise ReachedPdfBuild

        with (
            patch("build_manager.subprocess.run"),
            patch.object(self.manager, "_merge_docs_with_i18n"),
            patch.object(self.manager, "_ensure_version_index"),
            patch.object(self.manager, "_generate_version_config"),
            patch.object(PDFGeneratorV2, "generate_pdf", inspect_pdf),
            self.assertRaises(ReachedPdfBuild),
        ):
            self.manager.build_docs_in_worktree(worktree, self.version())


if __name__ == "__main__":
    unittest.main()
