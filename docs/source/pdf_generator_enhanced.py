#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
增强版PDF生成器
支持多种PDF生成方式：Puppeteer、浏览器打印、LaTeX
自动选择最佳可用的生成方式
"""

import os
import sys
import json
import subprocess
import tempfile
import shutil
import time
import webbrowser
import platform
from pathlib import Path
from typing import List, Dict, Optional, Union
import argparse
import http.server
import socketserver
import threading

class EnhancedPDFGenerator:
    """增强版PDF生成器"""
    
    def __init__(self, html_dir: Path, output_dir: Path):
        self.html_dir = html_dir
        self.output_dir = output_dir
        self.temp_dir = Path(tempfile.mkdtemp())
        self.merged_html = self.temp_dir / "merged.html"
        self.server_port = 8080
        self.server_thread = None
        self.server_running = False
        
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
        .print-instructions {{
            background: #fff3cd;
            border: 1px solid #ffeaa7;
            padding: 15px;
            margin: 20px 0;
            border-radius: 5px;
            text-align: center;
        }}
        @media print {{
            .print-instructions {{
                display: none;
            }}
            .toc {{
                page-break-after: always;
            }}
            section {{
                page-break-inside: avoid;
            }}
        }}
    </style>
    <script>
        // 自动打印功能
        function autoPrint() {{
            // 等待页面加载完成
            setTimeout(() => {{
                window.print();
            }}, 2000);
        }}
        
        // 页面加载完成后自动打印
        window.onload = function() {{
            // 检查URL参数
            const urlParams = new URLSearchParams(window.location.search);
            if (urlParams.get('print') === 'true') {{
                autoPrint();
            }}
        }};
    </script>
</head>
<body>
    <div class="print-instructions">
        <h3>📄 PDF生成说明</h3>
        <p>请使用浏览器的打印功能（Ctrl+P）将此页面保存为PDF</p>
        <p>或者点击 <a href="?print=true">自动打印</a> 按钮</p>
    </div>
    
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
    
    def check_puppeteer_available(self) -> bool:
        """检查Puppeteer是否可用"""
        try:
            # 检查Node.js
            result = subprocess.run(['node', '--version'], 
                                  capture_output=True, text=True, shell=True)
            if result.returncode != 0:
                return False
            
            # 检查Puppeteer
            result = subprocess.run(['npx', 'puppeteer', '--version'], 
                                  capture_output=True, text=True, shell=True)
            return result.returncode == 0
            
        except Exception:
            return False
    
    def install_puppeteer(self) -> bool:
        """安装Puppeteer依赖"""
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
    
    def generate_pdf_puppeteer(self, output_filename: str = "sdk-docs.pdf") -> bool:
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
            print("正在使用Puppeteer生成PDF...")
            result = subprocess.run(['npx', 'puppeteer', '--version'], 
                                  capture_output=True, text=True, shell=True)
            if result.returncode != 0:
                print("Puppeteer未正确安装，尝试使用npx运行...")
                result = subprocess.run(['npx', 'puppeteer', str(puppeteer_script)], 
                                      capture_output=True, text=True, shell=True)
            else:
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
    
    def start_server(self):
        """启动本地HTTP服务器"""
        class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
            def __init__(self, *args, **kwargs):
                # 修复：使用外部类的temp_dir属性
                super().__init__(*args, directory=str(self.server.temp_dir), **kwargs)
            
            def end_headers(self):
                self.send_header('Access-Control-Allow-Origin', '*')
                super().end_headers()
        
        try:
            with socketserver.TCPServer(("", self.server_port), CustomHTTPRequestHandler) as httpd:
                # 修复：将服务器实例传递给处理器
                httpd.RequestHandlerClass.server = self
                print(f"✓ 启动本地服务器: http://localhost:{self.server_port}")
                self.server_running = True
                httpd.serve_forever()
        except Exception as e:
            print(f"✗ 启动服务器失败: {e}")
            self.server_running = False
    
    def generate_pdf_browser(self, output_filename: str = "sdk-docs.pdf") -> bool:
        """使用浏览器生成PDF"""
        try:
            # 启动服务器线程
            self.server_thread = threading.Thread(target=self.start_server, daemon=True)
            self.server_thread.start()
            
            # 等待服务器启动
            time.sleep(2)
            
            if not self.server_running:
                print("✗ 服务器启动失败")
                return False
            
            # 打开浏览器
            url = f"http://localhost:{self.server_port}/merged.html?print=true"
            print(f"正在打开浏览器: {url}")
            
            # 使用默认浏览器打开
            webbrowser.open(url)
            
            # 等待用户操作
            print("\n" + "="*60)
            print("📄 PDF生成说明:")
            print("1. 浏览器已自动打开合并的HTML页面")
            print("2. 页面会自动触发打印对话框")
            print("3. 在打印对话框中选择'保存为PDF'")
            print("4. 选择保存位置并点击保存")
            print("5. 完成后按回车键继续...")
            print("="*60)
            
            input("按回车键继续...")
            
            # 确保输出目录存在
            self.output_dir.mkdir(parents=True, exist_ok=True)
            output_path = self.output_dir / output_filename
            
            print(f"✓ PDF生成流程完成")
            print(f"📁 请将生成的PDF文件保存到: {output_path}")
            
            return True
                
        except Exception as e:
            print(f"✗ PDF生成过程出错: {e}")
            return False
    
    def check_latex_available(self) -> bool:
        """检查LaTeX是否可用"""
        try:
            # 检查常见的LaTeX工具
            tools = ['xelatex', 'pdflatex', 'latexmk', 'tectonic']
            for tool in tools:
                result = subprocess.run([tool, '--version'], 
                                      capture_output=True, text=True, shell=True)
                if result.returncode == 0:
                    print(f"✓ 找到LaTeX工具: {tool}")
                    return True
            return False
        except Exception:
            return False
    
    def generate_pdf_latex(self, output_filename: str = "sdk-docs.pdf") -> bool:
        """使用LaTeX生成PDF（改进版）"""
        try:
            # 确保输出目录存在
            self.output_dir.mkdir(parents=True, exist_ok=True)
            output_path = self.output_dir / output_filename
            
            # 读取合并的HTML内容
            if not self.merged_html.exists():
                print("✗ 合并HTML文件不存在")
                return False
            
            with open(self.merged_html, 'r', encoding='utf-8') as f:
                html_content = f.read()
            
            # 简单的HTML到LaTeX转换
            import re
            
            # 提取标题
            title_match = re.search(r'<title>(.*?)</title>', html_content)
            doc_title = title_match.group(1) if title_match else output_filename.replace('.pdf', '')
            
            # 提取内容（简化版）
            body_match = re.search(r'<body>(.*?)</body>', html_content, re.DOTALL)
            if body_match:
                body_content = body_match.group(1)
                # 移除HTML标签，保留文本内容
                text_content = re.sub(r'<[^>]+>', '', body_content)
                text_content = re.sub(r'\s+', ' ', text_content).strip()
            else:
                text_content = "文档内容"
            
            # 创建LaTeX文档
            latex_content = f'''\\documentclass[12pt]{{article}}
\\usepackage[utf8]{{inputenc}}
\\usepackage{{xcolor}}
\\usepackage{{geometry}}
\\usepackage{{hyperref}}
\\usepackage{{graphicx}}
\\usepackage{{listings}}
\\usepackage{{fancyhdr}}
\\usepackage{{xunicode}}
\\usepackage{{fontspec}}

\\setmainfont{{Times New Roman}}
\\setsansfont{{Arial}}

\\geometry{{margin=2cm}}
\\pagestyle{{fancy}}
\\fancyhf{{}}
\\fancyhead[L]{{SDK文档}}
\\fancyfoot[C]{{\\thepage}}

\\title{{{doc_title}}}
\\author{{SDK文档生成器}}
\\date{{\\today}}

\\begin{{document}}

\\maketitle

\\tableofcontents

\\newpage

\\section{{文档内容}}
{text_content[:2000]}...

\\section{{说明}}
本文档由SDK文档生成器自动生成。完整内容请访问在线文档。

\\end{{document}}'''
            
            latex_file = self.temp_dir / "document.tex"
            with open(latex_file, 'w', encoding='utf-8') as f:
                f.write(latex_content)
            
            # 尝试使用tectonic编译（更现代的工具）
            print("正在使用LaTeX生成PDF...")
            tools_to_try = ['tectonic', 'xelatex', 'pdflatex']
            
            for tool in tools_to_try:
                try:
                    if tool == 'tectonic':
                        result = subprocess.run([tool, str(latex_file)], 
                                              cwd=str(self.temp_dir), capture_output=True, text=True, shell=True, encoding='utf-8', errors='ignore')
                    else:
                        result = subprocess.run([tool, '-interaction=nonstopmode', str(latex_file)], 
                                              cwd=str(self.temp_dir), capture_output=True, text=True, shell=True, encoding='utf-8', errors='ignore')
                    
                    if result.returncode == 0:
                        # 查找生成的PDF
                        pdf_candidates = [
                            self.temp_dir / "document.pdf",
                            self.temp_dir / f"{latex_file.stem}.pdf"
                        ]
                        
                        for pdf_candidate in pdf_candidates:
                            if pdf_candidate.exists():
                                shutil.copy2(pdf_candidate, output_path)
                                print(f"✓ PDF生成成功: {output_path}")
                                print(f"使用工具: {tool}")
                                return True
                        
                        # 查找任何PDF文件
                        pdf_files = list(self.temp_dir.glob("*.pdf"))
                        if pdf_files:
                            shutil.copy2(pdf_files[0], output_path)
                            print(f"✓ PDF生成成功: {output_path}")
                            print(f"使用工具: {tool}")
                            return True
                    
                except Exception as e:
                    print(f"工具 {tool} 失败: {e}")
                    continue
            
            print("✗ 所有LaTeX工具都失败了")
            return False
                
        except Exception as e:
            print(f"✗ LaTeX生成过程出错: {e}")
            return False
    
    def generate(self, title: str = "SDK文档", method: str = "auto") -> bool:
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
        
        # 2. 创建合并HTML
        print("2. 创建合并HTML文件...")
        if not self.create_merged_html(html_files, title):
            return False
        
        # 3. 选择生成方法
        if method == "auto":
            # 自动选择最佳方法
            if self.check_latex_available():
                method = "latex"  # 优先使用LaTeX，因为更稳定
            elif self.check_puppeteer_available():
                method = "puppeteer"
            else:
                method = "browser"
        
        print(f"3. 使用 {method} 方法生成PDF...")
        
        # 4. 生成PDF
        success = False
        if method == "puppeteer":
            if not self.check_puppeteer_available():
                print("Puppeteer不可用，尝试安装...")
                if self.install_puppeteer():
                    success = self.generate_pdf_puppeteer()
                else:
                    print("Puppeteer安装失败，回退到浏览器方法")
                    success = self.generate_pdf_browser()
            else:
                success = self.generate_pdf_puppeteer()
        elif method == "latex":
            success = self.generate_pdf_latex()
        elif method == "browser":
            success = self.generate_pdf_browser()
        else:
            print(f"✗ 未知的生成方法: {method}")
            return False
        
        if success:
            print("=" * 60)
            print("PDF生成完成!")
            print(f"📁 输出位置: {self.output_dir}")
            print("=" * 60)
        
        return success

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="增强版PDF生成器")
    parser.add_argument('--html-dir', type=str, default='source_build/html/latest',
                       help='HTML文件目录 (默认: source_build/html/latest)')
    parser.add_argument('--output-dir', type=str, default='source_build/html/latest/_static',
                       help='PDF输出目录 (默认: source_build/html/latest/_static)')
    parser.add_argument('--title', type=str, default='SDK文档',
                       help='PDF标题 (默认: SDK文档)')
    parser.add_argument('--method', type=str, default='auto',
                       choices=['auto', 'puppeteer', 'browser', 'latex'],
                       help='PDF生成方法 (默认: auto)')
    
    args = parser.parse_args()
    
    # 转换为Path对象
    html_dir = Path(args.html_dir)
    output_dir = Path(args.output_dir)
    
    if not html_dir.exists():
        print(f"✗ HTML目录不存在: {html_dir}")
        sys.exit(1)
    
    # 创建PDF生成器
    generator = EnhancedPDFGenerator(html_dir, output_dir)
    
    # 生成PDF
    success = generator.generate(args.title, args.method)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
