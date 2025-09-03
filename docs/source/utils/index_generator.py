#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
索引生成器模块
负责生成各种索引文件
"""

from pathlib import Path
import re
from typing import Dict, List


class IndexGenerator:
    def __init__(self, output_dir: str, file_processor):
        self.output_dir = Path(output_dir)
        self.file_processor = file_processor

    def generate_category_index(self, category: str, category_name: str, projects: List[str]) -> str:
        """生成分类索引页面"""
        title_length = len(category_name.encode('utf-8'))
        underline = '=' * title_length
        
        content = f"""{category_name}
{underline}

这里包含了 SDK 的 {category_name}。

.. toctree::
   :maxdepth: 2
   :caption: {category_name}

"""
        # 读取各项目的显示标题，用于自然排序（数字感知、大小写不敏感）
        items = []  # (display_title, project_path)
        for project in projects:
            display_title = self.file_processor.get_readme_title(project, category) or project
            items.append((display_title, project))

        # 自然排序函数
        def natural_key(s: str):
            return [int(part) if part.isdigit() else part.casefold() for part in re.split(r'(\d+)', s)]

        items.sort(key=lambda x: natural_key(x[0]))

        # 在 toctree 中使用“标题 <路径>”形式，展示更友好的名称
        for display_title, project in items:
            content += f"   {display_title} <{project}/README_zh>\n"
        
        content += f"\n这些示例展示了 SDK 的 {category_name}。\n"
        return content

    def generate_main_index(self, project_info: Dict) -> str:
        """生成主索引页面"""
        title = f"欢迎来到 {project_info.get('name', 'SDK')} 文档！"
        title_length = len(title.encode('utf-8'))
        underline = '=' * title_length

        content = f""".. {project_info.get('name', 'SDK')} documentation master file, created by sphinx-quickstart

{title}
{underline}

.. toctree::
   :maxdepth: 2
   :caption: 目录

   start/index
   basic/index
   driver/index
   component/index
   multimedia/index
   multcore/index

项目简介
--------
{project_info.get('description', '这里是 SDK 的简要介绍。')}
"""
        return content

    def write_index_file(self, content: str, file_path: str):
        """写入索引文件"""
        file_path = Path(file_path)
        file_path.parent.mkdir(parents=True, exist_ok=True)
        
        with open(file_path, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"已生成索引文件: {file_path}")

    def generate_all_indexes(self, categories: Dict, category_mapping: Dict, project_info: Dict):
        """生成所有索引文件"""
        # 生成主索引
        main_index_content = self.generate_main_index(project_info)
        self.write_index_file(main_index_content, self.output_dir / "index.rst")

        # 生成分类索引
        for category, config in categories.items():
            projects = category_mapping.get(category, [])
            if projects:
                category_name = config.get('name', category)
                index_content = self.generate_category_index(category, category_name, projects)
                index_path = self.output_dir / category / "index.rst"
                self.write_index_file(index_content, index_path) 