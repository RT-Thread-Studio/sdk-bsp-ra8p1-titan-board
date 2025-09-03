#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
项目扫描器模块
负责扫描和分类项目目录
"""

import os
import fnmatch
from pathlib import Path
from typing import Dict, List, Tuple


class ProjectScanner:
    def __init__(self, projects_dir: str, categories: Dict):
        self.projects_dir = Path(projects_dir)
        self.categories = categories
        self.category_mapping = {cat: [] for cat in categories.keys()}

    def scan_projects(self) -> Dict[str, List[str]]:
        """扫描项目目录并分类

        - 对于不含路径分隔符的 pattern（如: "Titan_basic_*"），仅在一级目录名上匹配。
        - 对于包含路径分隔符的 pattern（如: "dir/sub"），使用基于根目录的 glob 匹配精确目录。
        不会把更深层的子目录当作项目条目，严格以匹配到的目录为准。
        """
        if not self.projects_dir.exists():
            raise FileNotFoundError(f"项目目录不存在: {self.projects_dir}")

        print("开始扫描项目...")

        # 先缓存一级目录列表，便于无斜杠模式匹配
        top_level_dirs = [p for p in self.projects_dir.iterdir() if p.is_dir()]

        # 清空旧映射
        self.category_mapping = {cat: [] for cat in self.categories.keys()}

        for category, config in self.categories.items():
            patterns = config.get('patterns', [])
            matched_paths = []

            for pattern in patterns:
                # 标准化分隔符判断
                has_sep = ('/' in pattern) or ('\\' in pattern)
                if not has_sep:
                    # 仅在一级目录名上匹配
                    for d in top_level_dirs:
                        if fnmatch.fnmatch(d.name, pattern):
                            matched_paths.append(d)
                else:
                    # 使用根目录下的 glob 精确匹配相对路径
                    # 注意: Path.glob 接受 POSIX 风格的分隔符
                    for p in self.projects_dir.glob(pattern):
                        if p.is_dir():
                            matched_paths.append(p)

            # 去重并排序，填充分类结果
            seen = set()
            for p in matched_paths:
                rel = p.relative_to(self.projects_dir).as_posix()
                if rel not in seen:
                    seen.add(rel)
                    self.category_mapping[category].append(rel)
                    print(f"分类项目: {rel} -> {category}")

        return self.category_mapping

    def _classify_project(self, project_relative_path: str) -> str:
        """已不使用：分类逻辑已内联在 scan_projects 中。保留以兼容旧接口。"""
        for category, projects in self.category_mapping.items():
            if project_relative_path in projects:
                return category
        return None

    def get_projects_by_category(self, category: str) -> List[str]:
        """获取指定分类的项目列表"""
        return self.category_mapping.get(category, [])

    def get_all_projects(self) -> List[str]:
        """获取所有项目"""
        all_projects = []
        for projects in self.category_mapping.values():
            all_projects.extend(projects)
        return all_projects

    def validate_projects(self) -> bool:
        """验证项目目录结构"""
        for project_name in self.get_all_projects():
            project_path = self.projects_dir / project_name
            if not project_path.exists():
                print(f"警告: 项目目录不存在: {project_path}")
                return False
        return True 