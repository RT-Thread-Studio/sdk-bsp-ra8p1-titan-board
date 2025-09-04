#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PDF生成一键修复脚本
自动检测和修复PDF生成问题
"""

import os
import sys
import subprocess
import platform
from pathlib import Path

def check_node_js():
    """检查Node.js是否安装"""
    try:
        result = subprocess.run(['node', '--version'], 
                              capture_output=True, text=True, shell=True)
        if result.returncode == 0:
            print(f"✓ Node.js已安装: {result.stdout.strip()}")
            return True
        else:
            print("✗ Node.js未安装")
            return False
    except Exception:
        print("✗ Node.js未安装")
        return False

def install_node_js():
    """安装Node.js"""
    system = platform.system().lower()
    print(f"检测到系统: {system}")
    
    if system == 'windows':
        print("请手动安装Node.js:")
        print("1. 访问 https://nodejs.org/")
        print("2. 下载LTS版本")
        print("3. 运行安装程序")
        print("4. 重启命令行")
        return False
    elif system == 'linux':
        print("尝试使用包管理器安装Node.js...")
        try:
            # 尝试apt
            subprocess.run(['sudo', 'apt-get', 'update'], check=False)
            subprocess.run(['sudo', 'apt-get', 'install', '-y', 'nodejs', 'npm'], check=False)
            return check_node_js()
        except Exception:
            print("请手动安装Node.js: sudo apt-get install nodejs npm")
            return False
    elif system == 'darwin':
        print("尝试使用Homebrew安装Node.js...")
        try:
            subprocess.run(['brew', 'install', 'node'], check=False)
            return check_node_js()
        except Exception:
            print("请手动安装Node.js: brew install node")
            return False
    else:
        print(f"不支持的系统: {system}")
        return False

def check_puppeteer():
    """检查Puppeteer是否安装"""
    try:
        result = subprocess.run(['npx', 'puppeteer', '--version'], 
                              capture_output=True, text=True, shell=True)
        if result.returncode == 0:
            print(f"✓ Puppeteer已安装: {result.stdout.strip()}")
            return True
        else:
            print("✗ Puppeteer未安装")
            return False
    except Exception:
        print("✗ Puppeteer未安装")
        return False

def install_puppeteer():
    """安装Puppeteer"""
    try:
        print("正在安装Puppeteer...")
        subprocess.run(['npm', 'install', '-g', 'puppeteer'], 
                      check=True, shell=True)
        print("✓ Puppeteer安装成功")
        return True
    except subprocess.CalledProcessError as e:
        print(f"✗ Puppeteer安装失败: {e}")
        return False
    except Exception as e:
        print(f"✗ 安装过程出错: {e}")
        return False

def check_latex():
    """检查LaTeX是否安装"""
    tools = ['xelatex', 'pdflatex', 'latexmk', 'tectonic']
    for tool in tools:
        try:
            result = subprocess.run([tool, '--version'], 
                                  capture_output=True, text=True, shell=True)
            if result.returncode == 0:
                print(f"✓ 找到LaTeX工具: {tool}")
                return True
        except Exception:
            continue
    
    print("✗ 未找到LaTeX工具")
    return False

def install_latex():
    """安装LaTeX"""
    system = platform.system().lower()
    print(f"检测到系统: {system}")
    
    if system == 'windows':
        print("请手动安装MiKTeX:")
        print("1. 访问 https://miktex.org/")
        print("2. 下载并安装MiKTeX")
        print("3. 重启命令行")
        return False
    elif system == 'linux':
        print("尝试安装LaTeX...")
        try:
            subprocess.run(['sudo', 'apt-get', 'update'], check=False)
            subprocess.run(['sudo', 'apt-get', 'install', '-y', 
                          'texlive-latex-recommended', 
                          'texlive-latex-extra', 
                          'texlive-fonts-recommended', 
                          'texlive-xetex'], check=False)
            return check_latex()
        except Exception:
            print("请手动安装LaTeX: sudo apt-get install texlive-full")
            return False
    elif system == 'darwin':
        print("尝试安装MacTeX...")
        try:
            subprocess.run(['brew', 'install', 'basictex'], check=False)
            subprocess.run(['sudo', 'tlmgr', 'update', '--self'], check=False)
            subprocess.run(['sudo', 'tlmgr', 'install', 'latexmk', 'xetex'], check=False)
            return check_latex()
        except Exception:
            print("请手动安装MacTeX: brew install basictex")
            return False
    else:
        print(f"不支持的系统: {system}")
        return False

def fix_pdf_generation():
    """修复PDF生成问题"""
    print("=" * 60)
    print("PDF生成一键修复工具")
    print("=" * 60)
    
    # 1. 检查Node.js
    print("1. 检查Node.js...")
    if not check_node_js():
        print("需要安装Node.js")
        if not install_node_js():
            print("Node.js安装失败，请手动安装")
            return False
    
    # 2. 检查Puppeteer
    print("\n2. 检查Puppeteer...")
    if not check_puppeteer():
        print("需要安装Puppeteer")
        if not install_puppeteer():
            print("Puppeteer安装失败")
    
    # 3. 检查LaTeX（作为备选方案）
    print("\n3. 检查LaTeX...")
    if not check_latex():
        print("LaTeX未安装（可选，作为备选方案）")
        install_latex()
    
    # 4. 测试PDF生成
    print("\n4. 测试PDF生成...")
    try:
        from pdf_generator_enhanced import EnhancedPDFGenerator
        
        # 创建测试目录
        test_dir = Path("test_pdf")
        test_dir.mkdir(exist_ok=True)
        
        # 创建测试HTML文件
        test_html = test_dir / "test.html"
        with open(test_html, 'w', encoding='utf-8') as f:
            f.write('''<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>测试页面</title>
</head>
<body>
    <h1>PDF生成测试</h1>
    <p>这是一个测试页面，用于验证PDF生成功能。</p>
</body>
</html>''')
        
        # 测试PDF生成
        generator = EnhancedPDFGenerator(test_dir, test_dir)
        if generator.generate("测试文档", method="auto"):
            print("✓ PDF生成测试成功")
            
            # 清理测试文件
            import shutil
            shutil.rmtree(test_dir, ignore_errors=True)
            return True
        else:
            print("✗ PDF生成测试失败")
            return False
            
    except ImportError as e:
        print(f"✗ 无法导入PDF生成器: {e}")
        return False
    except Exception as e:
        print(f"✗ 测试过程出错: {e}")
        return False

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="PDF生成一键修复工具")
    parser.add_argument('--check-only', action='store_true', 
                       help='仅检查依赖，不安装')
    parser.add_argument('--install-node', action='store_true', 
                       help='安装Node.js')
    parser.add_argument('--install-puppeteer', action='store_true', 
                       help='安装Puppeteer')
    parser.add_argument('--install-latex', action='store_true', 
                       help='安装LaTeX')
    
    args = parser.parse_args()
    
    if args.check_only:
        print("检查依赖状态...")
        check_node_js()
        check_puppeteer()
        check_latex()
        return
    
    if args.install_node:
        install_node_js()
        return
    
    if args.install_puppeteer:
        install_puppeteer()
        return
    
    if args.install_latex:
        install_latex()
        return
    
    # 执行完整修复
    success = fix_pdf_generation()
    
    if success:
        print("\n🎉 PDF生成修复完成!")
        print("现在可以正常使用PDF生成功能了。")
    else:
        print("\n❌ PDF生成修复失败!")
        print("请检查错误信息并手动安装缺失的依赖。")
        sys.exit(1)

if __name__ == "__main__":
    main()
