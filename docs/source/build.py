#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简化的文档构建脚本
基于新的构建管理器，提供简单的构建接口
"""

import sys
import argparse
from pathlib import Path

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="文档构建工具")
    parser.add_argument('--clean', action='store_true', help='清理构建目录')
    parser.add_argument('--serve', action='store_true', help='启动本地服务器')
    parser.add_argument('--port', type=int, default=8000, help='服务器端口 (默认: 8000)')
    parser.add_argument('--validate', action='store_true', help='验证版本配置')
    parser.add_argument('--list-versions', action='store_true', help='列出所有版本')
    
    args = parser.parse_args()
    
    # 导入构建管理器
    try:
        from build_manager import BuildManager
    except ImportError:
        print("错误: 无法导入构建管理器")
        print("请确保 build_manager.py 文件存在")
        sys.exit(1)
    
    try:
        manager = BuildManager()
        
        if args.validate:
            from utils.version_utils import validate_versions_config
            success = validate_versions_config()
            sys.exit(0 if success else 1)
        
        elif args.list_versions:
            from utils.version_utils import get_version_configs
            versions = get_version_configs()
            print("版本列表:")
            for version in versions:
                print(f"  - {version['display_name']} ({version['name']}) -> {version['branch']}")
            return
        
        # 构建所有版本
        success = manager.build_all_versions(clean=args.clean)
        
        if success:
            print("\n🎉 所有版本构建成功!")
            print(f"📁 文档位置: {manager.versions_dir}")
            
            if args.serve:
                print(f"\n🌐 启动本地服务器 (http://localhost:{args.port})...")
                import subprocess
                try:
                    subprocess.run([
                        sys.executable, '-m', 'http.server', str(args.port)
                    ], cwd=str(manager.versions_dir))
                except KeyboardInterrupt:
                    print("\n服务器已停止")
        else:
            print("\n❌ 部分版本构建失败!")
            sys.exit(1)
            
    except Exception as e:
        print(f"✗ 构建错误: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main() 