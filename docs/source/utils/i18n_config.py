#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
国际化配置管理器
处理中英文文档的配置、目录结构和内容生成
"""

import os
import re
import yaml
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass


@dataclass
class CategoryConfig:
    """分类配置"""
    name: str
    name_en: str
    description: str
    patterns: List[str]


@dataclass
class ProjectConfig:
    """项目配置"""
    name: str
    title: str
    description: str
    description_en: str
    version: str
    author: str
    copyright: str
    website: str


@dataclass
class I18nConfig:
    """国际化配置"""
    project: ProjectConfig
    categories: Dict[str, CategoryConfig]
    repository: Dict[str, Any]
    generation: Dict[str, Any]
    sphinx: Dict[str, Any]


class I18nConfigManager:
    """国际化配置管理器"""
    
    def __init__(self, config_path: Path):
        self.config_path = config_path
        self.config = self._load_config()
    
    def _load_config(self) -> I18nConfig:
        """加载配置文件"""
        with open(self.config_path, 'r', encoding='utf-8') as f:
            data = yaml.safe_load(f) or {}
        
        # 解析项目配置
        project_data = data.get('project', {})
        project = ProjectConfig(
            name=project_data.get('name', ''),
            title=project_data.get('title', ''),
            description=project_data.get('description', ''),
            description_en=project_data.get('description_en', ''),
            version=project_data.get('version', ''),
            author=project_data.get('author', ''),
            copyright=project_data.get('copyright', ''),
            website=project_data.get('website', '')
        )
        
        # 解析分类配置
        categories = {}
        for key, cat_data in data.get('categories', {}).items():
            categories[key] = CategoryConfig(
                name=cat_data.get('name', ''),
                name_en=cat_data.get('name_en', ''),
                description=cat_data.get('description', ''),
                patterns=cat_data.get('patterns', [])
            )
        
        return I18nConfig(
            project=project,
            categories=categories,
            repository=data.get('repository', {}),
            generation=data.get('generation', {}),
            sphinx=data.get('sphinx', {})
        )
    
    def get_category_name(self, category_key: str, language: str = 'zh') -> str:
        """获取分类名称"""
        if category_key not in self.config.categories:
            return category_key
        
        category = self.config.categories[category_key]
        if language == 'en':
            return category.name_en or category.name
        return category.name
    
    def get_project_config(self, language: str = 'zh') -> Dict[str, str]:
        """获取项目配置"""
        project = self.config.project
        if language == 'en':
            return {
                'name': project.name,
                'title': f"{project.name} Documentation",
                'description': project.description_en or project.description,
                'version': project.version,
                'author': project.author,
                'copyright': project.copyright,
                'website': project.website,
                'language': 'en'
            }
        else:
            return {
                'name': project.name,
                'title': project.title,
                'description': project.description,
                'version': project.version,
                'author': project.author,
                'copyright': project.copyright,
                'website': project.website,
                'language': 'zh_CN'
            }
    
    def get_sphinx_config(self, language: str = 'zh') -> Dict[str, Any]:
        """获取Sphinx配置"""
        base_config = self.config.sphinx.copy()
        
        if language == 'en':
            base_config.update({
                'language': 'en',
                'html_lang': 'en',
                'html_title': f"{self.config.project.name} Documentation",
                'html_short_title': self.config.project.name,
                'html_search_language': 'en',
                'html_search_options': {
                    'dict': 'en'
                }
            })
        else:
            base_config.update({
                'language': 'zh_CN',
                'html_lang': 'zh-CN',
                'html_title': self.config.project.title,
                'html_short_title': self.config.project.title,
                'html_search_language': 'zh',
                'html_search_options': {
                    'dict': 'zh'
                }
            })
        
        return base_config
    
    def get_toc_structure(self, language: str = 'zh') -> List[Dict[str, Any]]:
        """获取目录结构"""
        structure = []
        
        for category_key in self.config.generation.get('output_structure', []):
            if category_key in self.config.categories:
                category = self.config.categories[category_key]
                structure.append({
                    'key': category_key,
                    'name': self.get_category_name(category_key, language),
                    'description': category.description,
                    'patterns': category.patterns
                })
        
        return structure
    
    def get_file_suffix(self, language: str = 'zh') -> str:
        """获取文件后缀"""
        return '_zh' if language == 'zh' else ''
    
    def get_index_filename(self, language: str = 'zh') -> str:
        """获取索引文件名"""
        return f"index{self.get_file_suffix(language)}.rst"
    
    def get_html_filename(self, base_name: str, language: str = 'zh') -> str:
        """获取HTML文件名"""
        if language == 'zh':
            return f"{base_name}_zh.html"
        return f"{base_name}.html"
    
    def get_markdown_filename(self, base_name: str, language: str = 'zh') -> str:
        """获取Markdown文件名"""
        if language == 'zh':
            return f"{base_name}_zh.md"
        return f"{base_name}.md"
    
    def extract_headings_from_markdown(self, file_path: Path) -> List[Dict[str, Any]]:
        """从Markdown文件中提取标题结构"""
        headings = []
        
        if not file_path.exists():
            return headings
        
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # 匹配标题的正则表达式
        heading_pattern = r'^(#{1,6})\s+(.+)$'
        
        for line_num, line in enumerate(content.split('\n'), 1):
            match = re.match(heading_pattern, line.strip())
            if match:
                level = len(match.group(1))
                title = match.group(2).strip()
                
                # 生成锚点ID
                anchor_id = self._generate_anchor_id(title)
                
                headings.append({
                    'level': level,
                    'title': title,
                    'anchor_id': anchor_id,
                    'line_num': line_num
                })
        
        return headings
    
    def _generate_anchor_id(self, title: str) -> str:
        """生成锚点ID"""
        # 移除特殊字符，转换为小写，用连字符连接
        anchor_id = re.sub(r'[^\w\s-]', '', title.lower())
        anchor_id = re.sub(r'[-\s]+', '-', anchor_id)
        return anchor_id.strip('-')
    
    def generate_toc_content(self, category_key: str, language: str = 'zh') -> str:
        """生成目录内容"""
        category = self.config.categories.get(category_key)
        if not category:
            return ""
        
        category_name = self.get_category_name(category_key, language)
        
        # 查找该分类下的所有项目
        projects = []
        docs_dir = self.config_path.parent
        
        for pattern in category.patterns:
            # 在docs目录下查找匹配的项目
            for item in docs_dir.glob(f"**/{pattern}"):
                if item.is_dir():
                    # 查找README文件
                    readme_file = None
                    if language == 'zh':
                        readme_file = item / "README_zh.md"
                        if not readme_file.exists():
                            readme_file = item / "README.md"
                    else:
                        readme_file = item / "README.md"
                        if not readme_file.exists():
                            readme_file = item / "README_zh.md"
                    
                    if readme_file and readme_file.exists():
                        # 提取项目名称
                        project_name = self._extract_project_name(readme_file, language)
                        projects.append({
                            'name': project_name,
                            'path': readme_file.relative_to(docs_dir),
                            'headings': self.extract_headings_from_markdown(readme_file)
                        })
        
        # 生成目录内容
        toc_lines = [f"{category_name}", "=" * len(category_name), ""]
        
        for project in projects:
            toc_lines.append(f".. toctree::")
            toc_lines.append(f"   :maxdepth: 3")
            toc_lines.append(f"   :caption: {project['name']}")
            toc_lines.append(f"")
            toc_lines.append(f"   {project['path'].with_suffix('')}")
            toc_lines.append(f"")
        
        return "\n".join(toc_lines)
    
    def _extract_project_name(self, readme_file: Path, language: str = 'zh') -> str:
        """从README文件中提取项目名称"""
        try:
            with open(readme_file, 'r', encoding='utf-8') as f:
                first_line = f.readline().strip()
            
            # 移除Markdown标题标记
            if first_line.startswith('#'):
                return first_line.lstrip('#').strip()
            
            return first_line
        except Exception:
            return readme_file.parent.name
    
    def get_language_config(self, language: str = 'zh') -> Dict[str, Any]:
        """获取语言特定配置"""
        return {
            'language': language,
            'file_suffix': self.get_file_suffix(language),
            'index_filename': self.get_index_filename(language),
            'project_config': self.get_project_config(language),
            'sphinx_config': self.get_sphinx_config(language),
            'toc_structure': self.get_toc_structure(language)
        }


def main():
    """测试函数"""
    config_path = Path(__file__).parent.parent / 'config.yaml'
    manager = I18nConfigManager(config_path)
    
    print("中文配置:")
    zh_config = manager.get_language_config('zh')
    print(f"  语言: {zh_config['language']}")
    print(f"  文件后缀: {zh_config['file_suffix']}")
    print(f"  索引文件: {zh_config['index_filename']}")
    print(f"  项目名称: {zh_config['project_config']['title']}")
    
    print("\n英文配置:")
    en_config = manager.get_language_config('en')
    print(f"  语言: {en_config['language']}")
    print(f"  文件后缀: {en_config['file_suffix']}")
    print(f"  索引文件: {en_config['index_filename']}")
    print(f"  项目名称: {en_config['project_config']['title']}")
    
    print("\n分类结构:")
    for category in en_config['toc_structure']:
        print(f"  {category['key']}: {category['name']}")


if __name__ == "__main__":
    main()