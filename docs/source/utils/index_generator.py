#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
索引生成器模块
负责生成各种索引文件，支持中英双语
"""

from pathlib import Path
import re
from typing import Dict, List, Tuple


class IndexGenerator:
    def __init__(self, output_dir: str, file_processor):
        self.output_dir = Path(output_dir)
        self.file_processor = file_processor

    def check_available_languages(self, project: str, category: str) -> Tuple[bool, bool]:
        """检查项目支持的语言
        
        Returns:
            (has_chinese, has_english): 是否有中文和英文文档
        """
        try:
            # 获取项目在 projects 目录中的实际路径
            project_path = None
            projects_dir = getattr(self.file_processor, 'projects_dir', 'projects')
            
            # 尝试不同的路径组合
            possible_paths = [
                Path(projects_dir) / project,
                Path(projects_dir) / category / project,
            ]
            
            for path in possible_paths:
                if path.exists():
                    project_path = path
                    break
            
            if not project_path:
                # 回退：检查输出目录中的文件
                output_project_path = self.output_dir / category / project
                has_chinese = (output_project_path / "README_zh.md").exists()
                has_english = (output_project_path / "README.md").exists()
                return has_chinese, has_english
            
            has_chinese = (project_path / "README_zh.md").exists()
            has_english = (project_path / "README.md").exists()
            
            return has_chinese, has_english
            
        except Exception as e:
            print(f"检查语言支持时出错 {project}: {e}")
            # 回退到默认：假设只有中文
            return True, False

    def generate_category_index(self, category: str, category_name: str, projects: List[str], language: str = 'zh') -> str:
        """生成分类索引页面，支持双语"""
        title_length = len(category_name.encode('utf-8'))
        underline = '=' * title_length
        
        if language == 'en':
            description = f"This section contains SDK {category_name.lower()}."
        else:
            description = f"这里包含了 SDK 的 {category_name}。"
        
        content = f"""{category_name}
{underline}

{description}

.. toctree::
   :maxdepth: 4
   :caption: {category_name}

"""
        # 读取各项目的显示标题，用于自然排序（数字感知、大小写不敏感）
        items = []  # (display_title, project_path, project_name, has_chinese, has_english)
        for project in projects:
            has_chinese, has_english = self.check_available_languages(project, category)
            
            # 根据语言选择标题
            display_title = None
            if language == 'en' and has_english:
                display_title = self.file_processor.get_readme_title(project, category, 'en') or project
            elif language == 'zh' and has_chinese:
                display_title = self.file_processor.get_readme_title(project, category, 'zh') or project
            elif has_chinese:
                display_title = self.file_processor.get_readme_title(project, category, 'zh') or project
            elif has_english:
                display_title = self.file_processor.get_readme_title(project, category, 'en') or project
            else:
                display_title = project
                
            items.append((display_title, project, has_chinese, has_english))

        # 自然排序函数
        def natural_key(s: str):
            return [int(part) if part.isdigit() else part.casefold() for part in re.split(r'(\d+)', s)]

        items.sort(key=lambda x: natural_key(x[0]))

        # 在 toctree 中生成条目
        for display_title, project, has_chinese, has_english in items:
            if language == 'en':
                # 英文版：只显示英文链接
                if has_english:
                    content += f"   {display_title} <{project}/README>\n"
                elif has_chinese:
                    # 如果没有英文版，回退到中文版
                    content += f"   {display_title} <{project}/README_zh>\n"
            else:
                # 中文版：只显示中文链接
                if has_chinese:
                    content += f"   {display_title} <{project}/README_zh>\n"
                elif has_english:
                    # 如果没有中文版，回退到英文版
                    content += f"   {display_title} <{project}/README>\n"
        
        if language == 'en':
            content += f"\nThese examples demonstrate SDK {category_name.lower()}.\n"
        else:
            content += f"\n这些示例展示了 SDK 的 {category_name}。\n"
        return content

    def generate_main_index(self, project_info: Dict, language: str = 'zh') -> str:
        """生成主索引页面"""
        if language == 'en':
            title = f"Welcome to {project_info.get('name', 'SDK')} Documentation!"
        else:
            title = f"欢迎来到 {project_info.get('name', 'SDK')} 文档！"
        title_length = len(title.encode('utf-8'))
        underline = '=' * title_length

        # 读取 output_structure 以动态生成章节顺序
        output_structure = []
        # 从 FileProcessor 的配置读取 output_structure
        try:
            output_structure = ((self.file_processor.config.get('output_structure', [])) or [])
        except Exception:
            output_structure = []
        if not output_structure:
            # 回退到已有分类顺序
            output_structure = ['start', 'basic', 'driver', 'component', 'multimedia', 'multcore']
            try:
                self.structure_mode = 'hardcoded'
            except Exception:
                pass
        else:
            try:
                self.structure_mode = 'dynamic'
            except Exception:
                pass

        toc_lines = []
        for cat in output_structure:
            if language == 'en':
                # 英文版：引用无后缀的index文件
                toc_lines.append(f"   {cat}/index")
            else:
                # 中文版：引用带_zh后缀的index文件
                toc_lines.append(f"   {cat}/index_zh")

        toc_block = "\n".join(toc_lines)

        # 根据语言选择正确的描述
        if language == 'en':
            description = project_info.get('description_en') or project_info.get('description', 'This is a brief introduction to the SDK.')
        else:
            description = project_info.get('description', '这里是 SDK 的简要介绍。')

        content = f""".. {project_info.get('name', 'SDK')} documentation master file, created by sphinx-quickstart

{title}
{underline}

.. toctree::
   :maxdepth: 4
   :caption: {'Contents' if language == 'en' else '目录'}

{toc_block}

{'Project Overview' if language == 'en' else '项目简介'}
--------
{description}
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
        # 生成主索引（中文版）- 使用 _zh 后缀
        main_index_content = self.generate_main_index(project_info, 'zh')
        self.write_index_file(main_index_content, self.output_dir / "index_zh.rst")
        
        # 生成主索引（英文版）- 无后缀表示英文
        main_index_content_en = self.generate_main_index(project_info, 'en')
        self.write_index_file(main_index_content_en, self.output_dir / "index.rst")

        # 生成分类索引
        total_chinese = 0
        total_english = 0
        total_bilingual = 0
        
        for category, config in categories.items():
            projects = category_mapping.get(category, [])
            if projects:
                # 生成中文版索引 - 使用 _zh 后缀
                category_name_zh = config.get('name', category)
                index_content_zh = self.generate_category_index(category, category_name_zh, projects, 'zh')
                index_path_zh = self.output_dir / category / "index_zh.rst"
                self.write_index_file(index_content_zh, index_path_zh)
                
                # 生成英文版索引 - 无后缀表示英文
                category_name_en = config.get('name_en', category)
                index_content_en = self.generate_category_index(category, category_name_en, projects, 'en')
                index_path_en = self.output_dir / category / "index.rst"
                self.write_index_file(index_content_en, index_path_en)
                
                # 统计语言支持情况
                for project in projects:
                    has_chinese, has_english = self.check_available_languages(project, category)
                    if has_chinese and has_english:
                        total_bilingual += 1
                    elif has_chinese:
                        total_chinese += 1
                    elif has_english:
                        total_english += 1

        # 末尾总结日志
        mode = getattr(self, 'structure_mode', 'hardcoded')
        if mode == 'dynamic':
            print("索引结构生成模式: 动态 (来自 config.yaml:generation.output_structure)")
        else:
            print("索引结构生成模式: 硬编码回退 (未在 config.yaml 中找到 output_structure)")
            
        print(f"语言支持统计: 双语文档 {total_bilingual} 个, 仅中文 {total_chinese} 个, 仅英文 {total_english} 个")