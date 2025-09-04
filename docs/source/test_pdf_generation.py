#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
PDF生成测试脚本
快速测试PDF生成功能是否正常工作
"""

import sys
import subprocess
from pathlib import Path

def test_pdf_generation():
    """测试PDF生成功能"""
    print("=" * 60)
    print("PDF生成功能测试")
    print("=" * 60)
    
    # 检查是否存在HTML构建目录
    html_dir = Path("source_build/html/latest")
    if not html_dir.exists():
        print("✗ HTML构建目录不存在，请先运行文档构建")
        print("运行命令: python build_manager.py")
        return False
    
    # 检查HTML文件
    html_files = list(html_dir.rglob("*.html"))
    if not html_files:
        print("✗ 未找到HTML文件")
        return False
    
    print(f"✓ 找到 {len(html_files)} 个HTML文件")
    
    # 测试增强版PDF生成器
    try:
        print("\n测试增强版PDF生成器...")
        from pdf_generator_enhanced import EnhancedPDFGenerator
        
        output_dir = html_dir / "_static"
        output_dir.mkdir(exist_ok=True)
        
        generator = EnhancedPDFGenerator(html_dir, output_dir)
        
        # 测试自动方法选择
        if generator.generate("SDK文档测试", method="auto"):
            pdf_file = output_dir / "sdk-docs.pdf"
            if pdf_file.exists():
                print(f"✓ PDF生成成功: {pdf_file}")
                print(f"文件大小: {pdf_file.stat().st_size} 字节")
                return True
            else:
                print("✗ PDF文件未生成")
                return False
        else:
            print("✗ PDF生成失败")
            return False
            
    except ImportError as e:
        print(f"✗ 无法导入增强版PDF生成器: {e}")
        
        # 回退到简化版
        try:
            print("\n测试简化版PDF生成器...")
            from pdf_generator_simple import SimplePDFGenerator
            
            output_dir = html_dir / "_static"
            output_dir.mkdir(exist_ok=True)
            
            generator = SimplePDFGenerator(html_dir, output_dir)
            
            if generator.generate("SDK文档测试"):
                pdf_file = output_dir / "sdk-docs.pdf"
                if pdf_file.exists():
                    print(f"✓ PDF生成成功: {pdf_file}")
                    print(f"文件大小: {pdf_file.stat().st_size} 字节")
                    return True
                else:
                    print("✗ PDF文件未生成")
                    return False
            else:
                print("✗ PDF生成失败")
                return False
                
        except ImportError as e2:
            print(f"✗ 无法导入简化版PDF生成器: {e2}")
            return False
    
    except Exception as e:
        print(f"✗ 测试过程出错: {e}")
        return False

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="PDF生成功能测试")
    parser.add_argument('--build-first', action='store_true', 
                       help='先构建文档再测试')
    
    args = parser.parse_args()
    
    if args.build_first:
        print("先构建文档...")
        try:
            result = subprocess.run([sys.executable, 'build_manager.py'], 
                                  capture_output=True, text=True)
            if result.returncode != 0:
                print(f"文档构建失败: {result.stderr}")
                return False
            print("✓ 文档构建完成")
        except Exception as e:
            print(f"文档构建出错: {e}")
            return False
    
    # 测试PDF生成
    success = test_pdf_generation()
    
    if success:
        print("\n🎉 PDF生成功能测试通过!")
        print("PDF生成功能已正常工作。")
    else:
        print("\n❌ PDF生成功能测试失败!")
        print("请运行修复脚本: python fix_pdf_generation.py")
        sys.exit(1)

if __name__ == "__main__":
    main()
