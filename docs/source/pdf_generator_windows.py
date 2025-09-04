#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Windows专用PDF生成器
使用浏览器打印功能生成PDF，避免复杂的依赖问题
"""

import os
import sys
import tempfile
import shutil
import time
import webbrowser
from pathlib import Path
from typing import List
import http.server
import socketserver
import threading

class WindowsPDFGenerator:
    """Windows专用PDF生成器"""
    
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
            max-width: 1200px;
            margin: 0 auto;
        }}
        .toc {{
            background: #f5f5f5;
            padding: 20px;
            margin-bottom: 30px;
            border-radius: 5px;
            border: 1px solid #ddd;
        }}
        .toc h2 {{
            margin-top: 0;
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
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
            padding: 5px 10px;
            border-radius: 3px;
            display: block;
        }}
        .toc a:hover {{
            background: #e3f2fd;
            text-decoration: none;
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
            border: 1px solid #e9ecef;
        }}
        pre {{
            background: #f8f9fa;
            padding: 15px;
            border-radius: 5px;
            overflow-x: auto;
            margin: 20px 0;
            border: 1px solid #e9ecef;
        }}
        img {{
            max-width: 100%;
            height: auto;
            margin: 15px 0;
            border: 1px solid #ddd;
            border-radius: 3px;
        }}
        table {{
            border-collapse: collapse;
            width: 100%;
            margin: 20px 0;
            border: 1px solid #ddd;
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
        .print-instructions h3 {{
            margin-top: 0;
            color: #856404;
        }}
        .print-instructions p {{
            margin: 5px 0;
            color: #856404;
        }}
        .print-instructions a {{
            color: #856404;
            font-weight: bold;
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
            body {{
                margin: 0;
                padding: 15px;
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
        <p><strong>推荐设置：</strong>选择"保存为PDF"，页面设置为A4，边距为默认</p>
    </div>
    
    <div class="toc">
        <h2>📚 目录</h2>
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
    
    def start_server(self):
        """启动本地HTTP服务器"""
        class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
            def __init__(self, *args, **kwargs):
                # 使用外部类的temp_dir属性
                super().__init__(*args, directory=str(self.server.temp_dir), **kwargs)
            
            def end_headers(self):
                self.send_header('Access-Control-Allow-Origin', '*')
                super().end_headers()
        
        try:
            with socketserver.TCPServer(("", self.server_port), CustomHTTPRequestHandler) as httpd:
                # 将服务器实例传递给处理器
                httpd.RequestHandlerClass.server = self
                print(f"✓ 启动本地服务器: http://localhost:{self.server_port}")
                self.server_running = True
                httpd.serve_forever()
        except Exception as e:
            print(f"✗ 启动服务器失败: {e}")
            self.server_running = False
    
    def generate_pdf(self, output_filename: str = "sdk-docs.pdf") -> bool:
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
    
    def generate(self, title: str = "SDK文档") -> bool:
        """生成PDF的主流程"""
        print("=" * 60)
        print("开始生成PDF文档 (Windows专用)")
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
        
        # 3. 生成PDF
        print("3. 启动浏览器生成PDF...")
        if not self.generate_pdf():
            return False
        
        print("=" * 60)
        print("PDF生成完成!")
        print(f"📁 输出位置: {self.output_dir}")
        print("=" * 60)
        
        return True

def main():
    """主函数"""
    import argparse
    
    parser = argparse.ArgumentParser(description="Windows专用PDF生成器")
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
    generator = WindowsPDFGenerator(html_dir, output_dir)
    
    # 生成PDF
    success = generator.generate(args.title)
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()
