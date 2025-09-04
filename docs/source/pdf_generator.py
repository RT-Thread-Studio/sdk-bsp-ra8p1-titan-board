#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
基于Puppeteer的PDF生成器
将所有HTML页面合并成一个大页面，然后生成PDF
"""

import os
import sys
import json
import subprocess
import tempfile
import shutil
from pathlib import Path
from typing import List, Dict, Optional
import argparse

class PDFGenerator:
    """PDF生成器"""
    
    def __init__(self, html_dir: Path, output_dir: Path):
        self.html_dir = html_dir
        self.output_dir = output_dir
        self.temp_dir = Path(tempfile.mkdtemp())
        self.merged_html = self.temp_dir / "merged.html"
        
    def __del__(self):
        """清理临时文件"""
        if self.temp_dir.exists():
            shutil.rmtree(self.temp_dir, ignore_errors=True)
    
    def find_html_files(self) -> List[Path]:
        """查找所有HTML文件"""
        html_files = []
        for html_file in self.html_dir.rglob("*.html"):
            # 排除一些不需要的页面
            if any(exclude in str(html_file) for exclude in [
                'search.html', 'genindex.html', 'py-modindex.html'
            ]):
                continue
            html_files.append(html_file)
        return sorted(html_files)
    
    def extract_page_content(self, html_file: Path) -> str:
        """提取HTML页面的主要内容"""
        try:
            with open(html_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 简单的HTML解析，提取body内容
            # 这里使用简单的字符串处理，实际项目中可能需要更复杂的HTML解析
            start_tag = '<body'
            end_tag = '</body>'
            
            start_pos = content.find(start_tag)
            if start_pos == -1:
                return ""
            
            # 找到body标签的结束位置
            body_start = content.find('>', start_pos) + 1
            body_end = content.find(end_tag, body_start)
            
            if body_end == -1:
                return ""
            
            body_content = content[body_start:body_end].strip()
            
            # 移除script和style标签
            import re
            body_content = re.sub(r'<script[^>]*>.*?</script>', '', body_content, flags=re.DOTALL)
            body_content = re.sub(r'<style[^>]*>.*?</style>', '', body_content, flags=re.DOTALL)
            
            return body_content
            
        except Exception as e:
            print(f"警告: 无法读取 {html_file}: {e}")
            return ""
    
    def create_merged_html(self, html_files: List[Path], title: str = "SDK文档") -> bool:
        """创建合并的HTML文件"""
        try:
            # 创建目录
            toc_items = []
            content_sections = []
            
            for i, html_file in enumerate(html_files):
                # 生成章节ID
                chapter_id = f"chapter-{i+1}"
                
                # 提取页面标题
                page_title = html_file.stem
                if page_title == 'index':
                    page_title = '首页'
                elif page_title.endswith('_index'):
                    page_title = page_title[:-6].title()
                
                # 添加到目录
                toc_items.append(f'<li><a href="#{chapter_id}">{page_title}</a></li>')
                
                # 提取内容
                content = self.extract_page_content(html_file)
                if content:
                    section = f'''
                    <section id="{chapter_id}">
                        <h1>{page_title}</h1>
                        {content}
                    </section>
                    '''
                    content_sections.append(section)
            
            # 生成完整的HTML
            html_template = f'''<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{title}</title>
    <style>
        body {{
            font-family: "Microsoft YaHei", "SimSun", sans-serif;
            line-height: 1.6;
            margin: 0;
            padding: 20px;
            color: #333;
        }}
        .toc {{
            background: #f5f5f5;
            padding: 20px;
            margin-bottom: 30px;
            border-radius: 5px;
        }}
        .toc h2 {{
            margin-top: 0;
            color: #2c3e50;
        }}
        .toc ul {{
            list-style: none;
            padding-left: 0;
        }}
        .toc li {{
            margin: 8px 0;
        }}
        .toc a {{
            text-decoration: none;
            color: #3498db;
            font-weight: 500;
        }}
        .toc a:hover {{
            text-decoration: underline;
        }}
        section {{
            margin-bottom: 40px;
            page-break-inside: avoid;
        }}
        section h1 {{
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
            margin-bottom: 20px;
        }}
        h1, h2, h3, h4, h5, h6 {{
            color: #2c3e50;
            margin-top: 30px;
            margin-bottom: 15px;
        }}
        p {{
            margin-bottom: 15px;
        }}
        code {{
            background: #f8f9fa;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: "Consolas", "Monaco", monospace;
        }}
        pre {{
            background: #f8f9fa;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            margin: 20px 0;
        }}
        img {{
            max-width: 100%;
            height: auto;
            margin: 15px 0;
        }}
        table {{
            border-collapse: collapse;
            width: 100%;
            margin: 20px 0;
        }}
        th, td {{
            border: 1px solid #ddd;
            padding: 8px 12px;
            text-align: left;
        }}
        th {{
            background: #f8f9fa;
            font-weight: bold;
        }}
        @media print {{
            .toc {{
                page-break-after: always;
            }}
            section {{
                page-break-inside: avoid;
            }}
        }}
    </style>
</head>
<body>
    <div class="toc">
        <h2>目录</h2>
        <ul>
            {''.join(toc_items)}
        </ul>
    </div>
    
    {''.join(content_sections)}
</body>
</html>'''
            
            with open(self.merged_html, 'w', encoding='utf-8') as f:
                f.write(html_template)
            
            print(f"✓ 已创建合并HTML文件: {self.merged_html}")
            return True
            
        except Exception as e:
            print(f"✗ 创建合并HTML文件失败: {e}")
            return False
    
    def install_puppeteer(self) -> bool:
        """安装Puppeteer依赖"""
        try:
            # 检查是否已安装Node.js
            result = subprocess.run(['node', '--version'], 
                                  capture_output=True, text=True, shell=True)
            if result.returncode != 0:
                print("✗ 未找到Node.js，请先安装Node.js")
                return False
            
            print(f"✓ 找到Node.js: {result.stdout.strip()}")
            
            # 检查是否已安装Puppeteer
            result = subprocess.run(['npx', 'puppeteer', '--version'], 
                                  capture_output=True, text=True, shell=True)
            if result.returncode == 0:
                print(f"✓ Puppeteer已安装: {result.stdout.strip()}")
                return True
            
            # 安装Puppeteer
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
    
    def generate_pdf(self, output_filename: str = "sdk-docs.pdf") -> bool:
        """使用Puppeteer生成PDF"""
        try:
            # 确保输出目录存在
            self.output_dir.mkdir(parents=True, exist_ok=True)
            output_path = self.output_dir / output_filename
            
            # 创建Puppeteer脚本
            puppeteer_script = self.temp_dir / "generate_pdf.js"
            
            script_content = f'''
const puppeteer = require('puppeteer');
const fs = require('fs');
const path = require('path');

async function generatePDF() {{
    const browser = await puppeteer.launch({{
        headless: true,
        args: ['--no-sandbox', '--disable-setuid-sandbox']
    }});
    
    const page = await browser.newPage();
    
    // 设置视口大小
    await page.setViewport({{
        width: 1200,
        height: 800
    }});
    
    // 加载HTML文件
    const htmlPath = path.resolve('{self.merged_html}');
    await page.goto(`file://${{htmlPath}}`, {{
        waitUntil: 'networkidle0',
        timeout: 30000
    }});
    
    // 等待内容加载完成
    await page.waitForTimeout(2000);
    
    // 生成PDF
    await page.pdf({{
        path: '{output_path}',
        format: 'A4',
        margin: {{
            top: '2cm',
            right: '2cm',
            bottom: '2cm',
            left: '2cm'
        }},
        displayHeaderFooter: true,
        headerTemplate: '<div style="font-size: 10px; text-align: center; width: 100%; color: #666;">SDK文档</div>',
        footerTemplate: '<div style="font-size: 10px; text-align: center; width: 100%; color: #666;"><span class="pageNumber"></span> / <span class="totalPages"></span></div>',
        printBackground: true
    }});
    
    await browser.close();
    console.log('PDF生成完成:', '{output_path}');
}}

generatePDF().catch(console.error);
'''
            
            with open(puppeteer_script, 'w', encoding='utf-8') as f:
                f.write(script_content)
            
            # 运行Puppeteer脚本
            print("正在生成PDF...")
            result = subprocess.run(['node', str(puppeteer_script)], 
                                  capture_output=True, text=True, cwd=self.temp_dir, shell=True)
            
            if result.returncode == 0:
                print(f"✓ PDF生成成功: {output_path}")
                return True
            else:
                print(f"✗ PDF生成失败: {result.stderr}")
                return False
                
        except Exception as e:
            print(f"✗ PDF生成过程出错: {e}")
            return False
    
    def generate(self, title: str = "SDK文档") -> bool:
        """生成PDF的主流程"""
        print("=" * 60)
        print("开始生成PDF文档")
        print("=" * 60)
        
        # 1. 查找HTML文件
        print("1. 查找HTML文件...")
        html_files = self.find_html_files()
        if not html_files:
            print("✗ 未找到HTML文件")
            return False
        print(f"✓ 找到 {len(html_files)} 个HTML文件")
        
        # 2. 安装Puppeteer
        print("2. 检查Puppeteer依赖...")
        if not self.install_puppeteer():
            return False
        
        # 3. 创建合并HTML
        print("3. 创建合并HTML文件...")
        if not self.create_merged_html(html_files, title):
            return False
        
        # 4. 生成PDF
        print("4. 生成PDF文件...")
        if not self.generate_pdf():
            return False
        
        print("=" * 60)
        print("PDF生成完成!")
        print(f"📁 输出位置: {self.output_dir}")
        print("=" * 60)
        
        return True

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="PDF生成器")
    parser.add_argument('--html-dir', type=str, default='source_build/html/latest',
                       help='HTML文件目录 (默认: source_build/html/latest)')
    parser.add_argument('--output-dir', type=str, default='source_build/html/latest/_static',
                       help='PDF输出目录 (默认: source_build/html/latest/_static)')
    parser.add_argument('--title', type=str, default='SDK文档',
                       help='PDF标题 (默认: SDK文档)')
    
    args = parser.parse_args()
    
    # 转换为Path对象
    html_dir = Path(args.html_dir)
    output_dir = Path(args.output_dir)
    
    if not html_dir.exists():
        print(f"✗ HTML目录不存在: {html_dir}")
        sys.exit(1)
    
    # 创建PDF生成器
    generator = PDFGenerator(html_dir, output_dir)
    
    # 生成PDF
    success = generator.generate(args.title)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
