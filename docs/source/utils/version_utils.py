#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
版本工具脚本
提供版本相关的通用功能
"""

import json
import subprocess
from pathlib import Path
from typing import List, Dict, Optional

def load_versions_config(project_root: Path = None) -> Dict:
    """加载版本配置文件"""
    if project_root is None:
        project_root = Path.cwd()
        while project_root != project_root.parent:
            if (project_root / '.github' / 'versions.json').exists():
                break
            project_root = project_root.parent
        else:
            raise FileNotFoundError("找不到 .github/versions.json 文件")
    
    versions_file = project_root / '.github' / 'versions.json'
    try:
        with open(versions_file, 'r', encoding='utf-8') as f:
            config = json.load(f)
        return config
    except Exception as e:
        print(f"错误: 无法加载版本配置: {e}")
        return {'versions': [], 'default_version': '', 'latest_version': ''}

def get_version_configs(project_root: Path = None) -> List[Dict]:
    """获取版本配置列表"""
    config = load_versions_config(project_root)
    return config.get('versions', [])

def get_current_branch() -> Optional[str]:
    """获取当前分支名称"""
    try:
        result = subprocess.run(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            capture_output=True, text=True, check=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return None

def get_branch_for_version(version_name: str, project_root: Path = None) -> Optional[str]:
    """根据版本名称获取对应的分支"""
    versions = get_version_configs(project_root)
    for version in versions:
        if version['name'] == version_name:
            return version['branch']
    return None

def get_version_for_branch(branch_name: str, project_root: Path = None) -> Optional[Dict]:
    """根据分支名称获取对应的版本配置"""
    versions = get_version_configs(project_root)
    for version in versions:
        if version['branch'] == branch_name:
            return version
    return None

def validate_versions_config(project_root: Path = None) -> bool:
    """验证版本配置的有效性"""
    try:
        config = load_versions_config(project_root)
        versions = config.get('versions', [])
        
        if not versions:
            print("错误: 没有找到版本配置")
            return False
        
        # 检查必需字段
        required_fields = ['name', 'display_name', 'branch', 'url_path']
        for i, version in enumerate(versions):
            for field in required_fields:
                if field not in version:
                    print(f"错误: 版本 {i+1} 缺少必需字段 '{field}'")
                    return False
        
        # 检查分支是否存在
        for version in versions:
            branch = version['branch']
            try:
                result = subprocess.run(
                    ['git', 'rev-parse', '--verify', f'refs/heads/{branch}'],
                    capture_output=True, check=True
                )
            except subprocess.CalledProcessError:
                print(f"警告: 分支 '{branch}' 不存在")
        
        # 检查默认版本
        default_version = config.get('default_version')
        if default_version:
            default_exists = any(v['name'] == default_version for v in versions)
            if not default_exists:
                print(f"警告: 默认版本 '{default_version}' 不在版本列表中")
        
        # 检查最新版本
        latest_version = config.get('latest_version')
        if latest_version:
            latest_exists = any(v['name'] == latest_version for v in versions)
            if not latest_exists:
                print(f"警告: 最新版本 '{latest_version}' 不在版本列表中")
        
        print(f"✓ 版本配置验证通过: {len(versions)} 个版本")
        return True
        
    except Exception as e:
        print(f"错误: 版本配置验证失败: {e}")
        return False

def create_version_redirect_html(version_config: Dict, target_url: str) -> str:
    """创建版本重定向页面"""
    return f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{version_config['display_name']} - SDK 文档</title>
    <meta http-equiv="refresh" content="0; url={target_url}">
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
        }}
        .container {{
            text-align: center;
            background: rgba(255, 255, 255, 0.1);
            padding: 40px;
            border-radius: 12px;
            backdrop-filter: blur(10px);
        }}
        .spinner {{
            border: 3px solid rgba(255, 255, 255, 0.3);
            border-top: 3px solid white;
            border-radius: 50%;
            width: 40px;
            height: 40px;
            animation: spin 1s linear infinite;
            margin: 0 auto 20px;
        }}
        @keyframes spin {{
            0% {{ transform: rotate(0deg); }}
            100% {{ transform: rotate(360deg); }}
        }}
        h1 {{
            margin: 0 0 10px 0;
            font-size: 24px;
        }}
        p {{
            margin: 0;
            opacity: 0.9;
        }}
        a {{
            color: white;
            text-decoration: underline;
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="spinner"></div>
        <h1>{version_config['display_name']}</h1>
        <p>正在跳转到文档...</p>
        <p><a href="{target_url}">如果页面没有自动跳转，请点击这里</a></p>
    </div>
</body>
</html>"""

if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description="版本工具")
    parser.add_argument('--validate', action='store_true', help='验证版本配置')
    parser.add_argument('--list', action='store_true', help='列出所有版本')
    parser.add_argument('--current-branch', action='store_true', help='获取当前分支')
    parser.add_argument('--version-for-branch', type=str, help='根据分支获取版本')
    parser.add_argument('--branch-for-version', type=str, help='根据版本获取分支')
    
    args = parser.parse_args()
    
    if args.validate:
        success = validate_versions_config()
        exit(0 if success else 1)
    
    elif args.list:
        versions = get_version_configs()
        print("版本列表:")
        for version in versions:
            print(f"  - {version['display_name']} ({version['name']}) -> {version['branch']}")
    
    elif args.current_branch:
        branch = get_current_branch()
        if branch:
            print(branch)
        else:
            print("无法获取当前分支")
            exit(1)
    
    elif args.version_for_branch:
        version = get_version_for_branch(args.version_for_branch)
        if version:
            print(json.dumps(version, ensure_ascii=False, indent=2))
        else:
            print(f"分支 '{args.version_for_branch}' 没有对应的版本配置")
            exit(1)
    
    elif args.branch_for_version:
        branch = get_branch_for_version(args.branch_for_version)
        if branch:
            print(branch)
        else:
            print(f"版本 '{args.branch_for_version}' 没有对应的分支")
            exit(1)
    
    else:
        parser.print_help() 