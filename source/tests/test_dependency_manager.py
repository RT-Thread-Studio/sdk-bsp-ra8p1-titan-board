import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import utils.dependency_manager as dependency_manager


class DependencyManagerTests(unittest.TestCase):
    def test_explicit_mirror_avoids_network_probe(self):
        with patch.dict(
            dependency_manager.os.environ,
            {"DOCS_PIP_MIRROR": "tsinghua"},
            clear=True,
        ), patch.object(dependency_manager, "rank_package_indexes") as rank:
            indexes = dependency_manager._configured_indexes(timeout=1)

        self.assertEqual([index.name for index in indexes], ["清华大学镜像"])
        rank.assert_not_called()

    def test_auto_mode_preserves_latency_order(self):
        ranked = [
            (dependency_manager.PACKAGE_INDEXES[1], 0.08),
            (dependency_manager.PACKAGE_INDEXES[0], 0.31),
            (dependency_manager.PACKAGE_INDEXES[2], 0.42),
        ]
        with patch.dict(dependency_manager.os.environ, {}, clear=True), patch.object(
            dependency_manager, "rank_package_indexes", return_value=ranked
        ):
            indexes = dependency_manager._configured_indexes(timeout=1)

        self.assertEqual(
            [index.name for index in indexes],
            ["清华大学镜像", "PyPI 官方源", "阿里云镜像"],
        )

    def test_pip_environment_index_keeps_detected_fallbacks(self):
        ranked = [(dependency_manager.PACKAGE_INDEXES[1], 0.08)]
        with patch.dict(
            dependency_manager.os.environ,
            {"PIP_INDEX_URL": "https://packages.example/simple/"},
            clear=True,
        ), patch.object(
            dependency_manager, "rank_package_indexes", return_value=ranked
        ):
            indexes = dependency_manager._configured_indexes(timeout=1)

        self.assertEqual(indexes[0].name, "pip 环境配置源")
        self.assertEqual(indexes[1].name, "清华大学镜像")

    def test_install_falls_back_to_next_reachable_index(self):
        first, second = dependency_manager.PACKAGE_INDEXES[:2]
        completed = [
            subprocess.CompletedProcess([], returncode=1),
            subprocess.CompletedProcess([], returncode=0),
        ]
        with tempfile.TemporaryDirectory() as temp_dir:
            requirements = Path(temp_dir) / "requirements.txt"
            requirements.write_text("sphinx\n", encoding="utf-8")
            with patch.object(dependency_manager, "_ensure_pip", return_value=True), patch.object(
                dependency_manager,
                "_configured_indexes",
                return_value=[first, second],
            ), patch.object(
                dependency_manager.subprocess, "run", side_effect=completed
            ) as run, patch.object(
                dependency_manager, "find_dependency_issues", return_value={}
            ):
                installed = dependency_manager.install_dependencies(requirements)

        self.assertTrue(installed)
        self.assertEqual(run.call_count, 2)
        self.assertIn(first.url, run.call_args_list[0].args[0])
        self.assertIn(second.url, run.call_args_list[1].args[0])

    def test_pinned_version_mismatch_is_reported(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            requirements = Path(temp_dir) / "requirements.txt"
            requirements.write_text("Sphinx==1.2.3\n", encoding="utf-8")
            with patch.object(
                dependency_manager.importlib_metadata,
                "version",
                return_value="9.9.9",
            ):
                issues = dependency_manager.find_dependency_issues(requirements)

        self.assertEqual(issues["Sphinx"], "installed 9.9.9, required 1.2.3")


if __name__ == "__main__":
    unittest.main()
