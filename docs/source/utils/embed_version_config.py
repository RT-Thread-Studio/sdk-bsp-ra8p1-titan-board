#!/usr/bin/env python3
"""
版本配置嵌入脚本
在构建过程中将 .github/versions.json 的内容嵌入到 version_menu.js 中
完全基于 .github/versions.json，无硬编码版本信息
支持自动备份和还原 version_menu.js
"""

import json
import os
import sys
from pathlib import Path
import shutil

def load_versions_config():
    """加载 .github/versions.json 文件"""
    versions_file = Path(__file__).parent.parent.parent.parent / '.github' / 'versions.json'
    if not versions_file.exists():
        print(f"警告: 找不到版本配置文件 {versions_file}")
        return None
    try:
        with open(versions_file, 'r', encoding='utf-8') as f:
            config = json.load(f)
            if not isinstance(config, dict) or 'versions' not in config:
                print(f"错误: 版本配置文件格式不正确 {versions_file}")
                return None
            return config
    except Exception as e:
        print(f"错误: 无法读取版本配置文件: {e}")
        return None

def embed_config_to_js(versions_config):
    if not versions_config:
        return """
    // 获取嵌入的版本配置（在构建时由脚本生成）
    function getEmbeddedVersionConfig() {
        return null; // 无可用版本配置
    }
    """
    
    # 确保配置格式正确
    config_js = json.dumps(versions_config, ensure_ascii=False, indent=2)
    embedded_config_js = f"""
    // 获取嵌入的版本配置（在构建时由脚本生成）
    function getEmbeddedVersionConfig() {{
        return {config_js};
    }}
    """
    return embedded_config_js

def backup_js(js_file):
    bak_file = js_file.with_suffix('.js.bak')
    if js_file.exists():
        shutil.copy2(js_file, bak_file)
        print(f"已备份 {js_file} -> {bak_file}")
    return bak_file

def restore_js(js_file):
    bak_file = js_file.with_suffix('.js.bak')
    if bak_file.exists():
        shutil.copy2(bak_file, js_file)
        print(f"已还原 {bak_file} -> {js_file}")
        return True
    else:
        print(f"未找到备份文件 {bak_file}")
        return False

def update_version_menu_js(restore_after=False):
    versions_config = load_versions_config()
    embedded_config_js = embed_config_to_js(versions_config)
    js_file = Path(__file__).parent.parent / '_static' / 'version_menu.js'
    
    # 备份
    bak_file = backup_js(js_file)
    try:
        with open(js_file, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"错误: 无法读取 {js_file}: {e}")
        return False
    
    # 使用括号深度匹配方式安全替换函数体，避免 JSON 中的 '}' 提前终止
    start_marker = '// 获取嵌入的版本配置（在构建时由脚本生成）'
    func_sig = 'function getEmbeddedVersionConfig()'
    start_idx = content.find(start_marker)
    replaced = False
    if start_idx != -1:
        sig_idx = content.find(func_sig, start_idx)
        if sig_idx != -1:
            brace_start = content.find('{', sig_idx)
            if brace_start != -1:
                depth = 0
                i = brace_start
                while i < len(content):
                    ch = content[i]
                    if ch == '{':
                        depth += 1
                    elif ch == '}':
                        depth -= 1
                        if depth == 0:
                            # 包含右大括号本身
                            end_idx = i + 1
                            # 用嵌入代码整体替换这段函数定义
                            content = content[:start_idx] + embedded_config_js.strip() + content[end_idx:]
                            replaced = True
                            print("替换现有嵌入配置函数（括号深度）")
                            break
                    i += 1
    if not replaced:
        # 查找插入位置 - 在 fetchVersionInfo 函数之前插入
        fetch_marker = 'async function fetchVersionInfo()'
        fetch_idx = content.find(fetch_marker)
        if fetch_idx != -1:
            content = content[:fetch_idx] + embedded_config_js.strip() + '\n\n    ' + content[fetch_idx:]
            print("在 fetchVersionInfo 函数前插入嵌入配置函数")
        else:
            # 如果找不到 fetchVersionInfo，在文件开头插入
            content = embedded_config_js.strip() + '\n\n' + content
            print("在文件开头插入嵌入配置函数")
    
    try:
        with open(js_file, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"成功更新 {js_file}")
        if versions_config:
            print(f"嵌入版本配置: {len(versions_config.get('versions', []))} 个版本")
            for version in versions_config.get('versions', []):
                print(f"  - {version['name']}: {version['display_name']} -> {version['url_path']}")
        else:
            print("嵌入兜底配置: 无可用版本")
    except Exception as e:
        print(f"错误: 无法写入 {js_file}: {e}")
        return False
    
    # 可选还原
    if restore_after:
        restore_js(js_file)
    return True

def main():
    restore_after = False
    if len(sys.argv) > 1 and sys.argv[1] == '--restore-after':
        restore_after = True
    
    print("开始嵌入版本配置...")
    if update_version_menu_js(restore_after=restore_after):
        print("版本配置嵌入完成")
        
        # 如果不是立即恢复，则在构建完成后自动恢复
        if not restore_after:
            print("注意: 构建完成后请手动运行 'python utils/embed_version_config.py --restore-after' 来恢复备份文件")
        
        return 0
    else:
        print("版本配置嵌入失败")
        return 1

if __name__ == '__main__':
    sys.exit(main()) 