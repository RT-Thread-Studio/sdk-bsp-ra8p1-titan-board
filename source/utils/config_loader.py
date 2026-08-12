#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
配置加载器模块
负责加载和验证配置文件
"""

import yaml
from pathlib import Path
from typing import Dict, Any


class ConfigLoader:
    def __init__(self, config_path: str = "config.yaml"):
        self.config_path = Path(config_path)
        self.config = None
        self._load_config()

    def _load_config(self):
        """加载配置文件"""
        if not self.config_path.exists():
            raise FileNotFoundError(f"配置文件不存在: {self.config_path}")
        
        with open(self.config_path, 'r', encoding='utf-8') as f:
            self.config = yaml.safe_load(f)

    def get_project_info(self) -> Dict[str, Any]:
        """获取项目信息"""
        return self.config.get('project', {})

    def get_repository_info(self) -> Dict[str, Any]:
        """获取仓库信息"""
        return self.config.get('repository', {})

    def get_categories(self) -> Dict[str, Any]:
        """获取分类配置"""
        return self.config.get('categories', {})

    def get_generation_config(self) -> Dict[str, Any]:
        """获取生成配置"""
        return self.config.get('generation', {})

    def get_sphinx_config(self) -> Dict[str, Any]:
        """获取Sphinx配置"""
        return self.config.get('sphinx', {})

    def resolve_repository_path(self, key: str, default: str) -> Path:
        """按 config.yaml 所在目录解析仓库路径配置。"""
        configured_path = self.get_repository_info().get(key, default)
        path = Path(configured_path)
        if not path.is_absolute():
            path = self.config_path.parent / path
        return path.resolve()

    def validate_config(self) -> bool:
        """验证配置文件"""
        required_sections = ['project', 'repository', 'categories', 'generation', 'sphinx']
        for section in required_sections:
            if section not in self.config:
                raise ValueError(f"配置文件缺少必需的部分: {section}")
        return True
