#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
检查构建结果脚本
用于GitHub Actions中验证构建是否成功
"""

import os
import json
from pathlib import Path

def check_build_results():
    """检查构建结果"""
    print("=== 检查生成的文档 ===")
    
    # 检查构建目录
    build_dir = Path("_build")
    if not build_dir.exists():
        print("✗ 构建目录不存在")
        return False
    
    # 检查版本目录
    versions_dir = build_dir / "versions"
    if not versions_dir.exists():
        print("✗ 版本目录不存在")
        return False
    
    print("✓ 版本目录存在")
    
    # 检查各个版本
    version_dirs = [d for d in versions_dir.iterdir() if d.is_dir()]
    if not version_dirs:
        print("✗ 没有找到版本目录")
        return False
    
    print(f"✓ 找到 {len(version_dirs)} 个版本目录")
    
    for version_dir in version_dirs:
        version_name = version_dir.name
        print(f"  检查版本: {version_name}")
        
        # 检查index.html
        index_file = version_dir / "index.html"
        if not index_file.exists():
            print(f"    ✗ {version_name}/index.html 不存在")
            return False
        print(f"    ✓ {version_name}/index.html 存在")
        
        # 检查version_config.json
        config_file = version_dir / "version_config.json"
        if not config_file.exists():
            print(f"    ✗ {version_name}/version_config.json 不存在")
            return False
        print(f"    ✓ {version_name}/version_config.json 存在")
        
        # 检查_static目录
        static_dir = version_dir / "_static"
        if not static_dir.exists():
            print(f"    ✗ {version_name}/_static 目录不存在")
            return False
        print(f"    ✓ {version_name}/_static 目录存在")
        
        # 检查version_menu.js
        js_file = static_dir / "version_menu.js"
        if not js_file.exists():
            print(f"    ✗ {version_name}/_static/version_menu.js 不存在")
            return False
        print(f"    ✓ {version_name}/_static/version_menu.js 存在")
        
        # 检查version_menu.css
        css_file = static_dir / "version_menu.css"
        if not css_file.exists():
            print(f"    ✗ {version_name}/_static/version_menu.css 不存在")
            return False
        print(f"    ✓ {version_name}/_static/version_menu.css 存在")
    
    # 检查根页面
    root_index = build_dir / "html" / "index.html"
    if root_index.exists():
        print("✓ 根目录重定向页面已生成")
    else:
        print("✗ 根目录重定向页面未生成")
        return False
    
    # 检查versions根页面
    versions_root_index = versions_dir / "index.html"
    if versions_root_index.exists():
        print("✓ versions目录根页面已生成")
    else:
        print("✗ versions目录根页面未生成")
        return False
    
    print("=== 所有检查通过 ===")
    return True

def main():
    """主函数"""
    success = check_build_results()
    if not success:
        print("构建检查失败")
        exit(1)
    else:
        print("构建检查成功")

if __name__ == "__main__":
    main() 