#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
增强版PDF生成器 V2
基于HTML文件解析和Markdown内容提取的PDF生成系统
支持中英文双语版本，自动生成目录和正文
"""

import os
import sys
import json
import re
import tempfile
import shutil
import subprocess
from pathlib import Path
from typing import List, Dict, Optional, Tuple
import argparse
from datetime import datetime
from bs4 import BeautifulSoup
import markdown
from markdown.extensions import codehilite, tables, toc

class HTMLParser:
    """HTML文件解析器"""
    
    def __init__(self, html_file: Path):
        self.html_file = html_file
        self.soup = None
        self._parse_html()
    
    def _parse_html(self):
        """解析HTML文件"""
        try:
            with open(self.html_file, 'r', encoding='utf-8') as f:
                content = f.read()
            self.soup = BeautifulSoup(content, 'html.parser')
        except Exception as e:
            print(f"警告: 无法解析HTML文件 {self.html_file}: {e}")
            self.soup = None
    
    def extract_title(self) -> str:
        """提取页面标题"""
        if not self.soup:
            return "未知标题"
        
        # 尝试多种方式获取标题
        title_selectors = [
            'h1',
            '.rst-content h1',
            'title',
            '.document h1'
        ]
        
        for selector in title_selectors:
            element = self.soup.select_one(selector)
            if element:
                title = element.get_text().strip()
                if title and title != "Titan-Board SDK 1.0.0 文档":
                    return title
        
        return "未知标题"
    
    def extract_content(self) -> str:
        """提取主要内容（保留HTML结构，并将资源链接改为绝对路径）"""
        if not self.soup:
            return ""
        
        # 查找主要内容区域
        content_selectors = [
            '.rst-content',
            '.document',
            '.body',
            'main',
            'article'
        ]
        
        content_element = None
        for selector in content_selectors:
            content_element = self.soup.select_one(selector)
            if content_element:
                break
        
        if not content_element:
            # 如果没有找到特定容器，使用body
            content_element = self.soup.find('body')
        
        if not content_element:
            return ""

        # 移除不需要的元素（脚本、样式、侧栏、版本菜单、编辑/PDF按钮、搜索框等）
        remove_selectors = [
            'script', 'style', 'nav',
            '.wy-nav-side', '.rst-versions', '.version-menu',
            '.edit-button', '.pdf-button', '[role="search"]',
            'a.headerlink'
        ]
        for sel in remove_selectors:
            for node in content_element.select(sel):
                node.decompose()

        # 将相对资源路径转换为绝对 file:// 路径
        base_dir = self.html_file.parent
        def _abs_url(url: str) -> str:
            if not url:
                return url
            u = url.strip()
            if u.startswith(('http://', 'https://', 'data:', 'mailto:', '#')):
                return u
            try:
                return (base_dir / u).resolve().as_uri()
            except Exception:
                return u

        for tag in content_element.select('[src]'):
            tag['src'] = _abs_url(tag.get('src'))
        for tag in content_element.select('a[href]'):
            href = tag.get('href')
            # 锚点保持不变
            if href and not href.startswith('#'):
                tag['href'] = _abs_url(href)

        # 返回主要内容的内部HTML，保留原有结构与样式类名
        return content_element.decode_contents()
    
    def extract_toc(self) -> List[Dict]:
        """提取目录结构"""
        if not self.soup:
            return []
        
        toc_items = []
        
        # 查找目录元素
        toc_selectors = [
            '.wy-menu .toctree-l1',
            '.toctree-wrapper .toctree-l1',
            '.rst-content .toctree-l1'
        ]
        
        for selector in toc_selectors:
            elements = self.soup.select(selector)
            if elements:
                for element in elements:
                    link = element.find('a')
                    if link:
                        title = link.get_text().strip()
                        href = link.get('href', '')
                        if title and href:
                            toc_items.append({
                                'title': title,
                                'href': href,
                                'level': 1
                            })
                break
        
        return toc_items

class MarkdownProcessor:
    """Markdown处理器"""
    
    def __init__(self):
        self.md = markdown.Markdown(
            extensions=[
                'codehilite',
                'tables',
                'toc',
                'fenced_code',
                'attr_list',
                'nl2br',
                'sane_lists',
                'footnotes'
            ],
            extension_configs={
                'codehilite': {
                    'css_class': 'highlight',
                    'use_pygments': False
                },
                'toc': {
                    'permalink': False,
                    'baselevel': 1
                }
            }
        )
    
    def process_markdown(self, md_content: str) -> str:
        """处理Markdown内容"""
        try:
            # Markdown 实例是有状态的，转换前需重置以避免交叉污染
            self.md.reset()
            html = self.md.convert(md_content)
            return html
        except Exception as e:
            print(f"警告: Markdown处理失败: {e}")
            return md_content

class DocumentScanner:
    """文档扫描器（从原始Markdown项目目录读取）"""
    
    def __init__(self, html_dir: Path, projects_root: Path):
        self.html_dir = html_dir
        self.projects_root = projects_root
        # 分类与顺序：从 config.yaml 的 generation.output_structure 读取，如果缺省则按默认顺序
        self.categories = {}
        self.category_name_map = {}
        self.category_order: List[str] = []
        try:
            cfg_path = Path(__file__).parent / 'config.yaml'
            if cfg_path.exists():
                import yaml
                with open(cfg_path, 'r', encoding='utf-8') as f:
                    cfg = yaml.safe_load(f) or {}
                cfg_cats = (cfg.get('categories') or {})
                out_struct = ((cfg.get('generation', {}) or {}).get('output_structure', []) or [])
                if out_struct:
                    self.category_order = list(out_struct)
                else:
                    self.category_order = ['start', 'basic', 'driver', 'component', 'multimedia', 'multcore']
                # 名称映射 + 填充 categories 字典用于扫描
                for key in self.category_order:
                    node = cfg_cats.get(key) or {}
                    name_cn = node.get('name') or key
                    name_en = node.get('name_en') or name_cn
                    self.category_name_map[key] = {'name': name_cn, 'name_en': name_en}
                    # categories 值用中文默认名作占位
                    self.categories[key] = name_cn
        except Exception:
            # 回退默认
            self.category_order = ['start', 'basic', 'driver', 'component', 'multimedia', 'multcore']
            for key, default_cn in {
                'start': '快速上手',
                'basic': '基础篇',
                'driver': '驱动篇',
                'component': '组件篇',
                'multimedia': '多媒体显示篇',
                'multcore': '多核通信篇'
            }.items():
                self.category_name_map[key] = {'name': default_cn, 'name_en': default_cn}
                self.categories[key] = default_cn
    
    def scan_documents(self, language: str = 'zh') -> Dict[str, List[Dict]]:
        """扫描文档结构（只使用 Markdown 文件）"""
        documents = {}
        
        for category, category_name in self.categories.items():
            # 用 html_dir 列出该分类下有哪些项目目录（递归），以确定项目名
            category_dir = self.html_dir / category
            if not category_dir.exists():
                continue
            
            category_docs = []
            
            # 递归查找该分类下的所有项目目录：判定标准为目录中存在任意 HTML（README*.html 等）
            try:
                candidates = []
                for html_file in category_dir.rglob('*.html'):
                    rel = html_file.parent.relative_to(category_dir)  # 支持多级路径
                    if str(rel) == '.':
                        continue
                    candidates.append(rel)
            except Exception:
                candidates = []

            # 去重并按路径稳定排序
            seen = set()
            uniq_candidates = []
            for rel in sorted(candidates, key=lambda p: str(p).lower()):
                key = str(rel).replace('\\','/')
                if key in seen:
                    continue
                seen.add(key)
                uniq_candidates.append(rel)

            for rel_path in uniq_candidates:
                project_dir = self.projects_root / rel_path
                if not project_dir.exists():
                    # 回退到最后一段匹配
                    project_dir = self.projects_root / rel_path.name
                    if not project_dir.exists():
                        continue

                readme_file = project_dir / ('README_zh.md' if language == 'zh' else 'README.md')
                if not readme_file.exists():
                    # 若指定语言缺失，尝试另一种
                    alt_readme = project_dir / ('README.md' if language == 'zh' else 'README_zh.md')
                    if alt_readme.exists():
                        readme_file = alt_readme
                    else:
                        continue

                title = self._extract_markdown_title(readme_file)
                category_docs.append({
                    'title': title,
                    'file': readme_file,
                    'project_name': str(rel_path).replace('\\','/'),
                    'project_dir': project_dir,
                    'category': category,
                    'category_name': self.category_name_map.get(category, {}).get('name', category),
                    'category_name_en': self.category_name_map.get(category, {}).get('name_en', self.category_name_map.get(category, {}).get('name', category))
                })

            if category_docs:
                documents[category] = category_docs
        
        return documents
    
    def _extract_markdown_title(self, md_file: Path) -> str:
        """从Markdown文件提取标题"""
        try:
            with open(md_file, 'r', encoding='utf-8') as f:
                content = f.read()
            
            # 查找第一个一级标题
            lines = content.split('\n')
            for line in lines:
                line = line.strip()
                if line.startswith('# '):
                    title = line[2:].strip()
                    # 清理标题中的Markdown格式
                    title = re.sub(r'\*\*(.*?)\*\*', r'\1', title)  # 移除粗体
                    title = re.sub(r'\*(.*?)\*', r'\1', title)      # 移除斜体
                    title = re.sub(r'`(.*?)`', r'\1', title)        # 移除代码格式
                    return title
            
            # 如果没有找到一级标题，尝试二级标题
            for line in lines:
                line = line.strip()
                if line.startswith('## '):
                    title = line[3:].strip()
                    title = re.sub(r'\*\*(.*?)\*\*', r'\1', title)
                    title = re.sub(r'\*(.*?)\*', r'\1', title)
                    title = re.sub(r'`(.*?)`', r'\1', title)
                    return title
            
            return md_file.stem
        except Exception as e:
            print(f"警告: 无法读取Markdown文件 {md_file}: {e}")
            return md_file.stem

class PDFGeneratorV2:
    """增强版PDF生成器V2"""
    
    def __init__(self, html_dir: Path, output_dir: Path, keep_temp: bool = False, browser_path: Optional[str] = None):
        self.html_dir = html_dir
        self.output_dir = output_dir
        self.temp_dir = Path(tempfile.mkdtemp())
        self.keep_temp = keep_temp
        self.browser_path = browser_path
        self.scanner = DocumentScanner(html_dir, self._derive_projects_root())
        self.md_processor = MarkdownProcessor()
        self.toc_entries = []  # [{'level':1,'title':'1. Title','anchor':'id'}]
        self.assets_dir: Optional[Path] = None
    def __del__(self):
        """清理临时文件"""
        try:
            if not getattr(self, 'keep_temp', False) and self.temp_dir.exists():
                shutil.rmtree(self.temp_dir, ignore_errors=True)
        except Exception:
            pass
    
    def generate_pdf(self, title: str = "SDK文档", language: str = "zh") -> bool:
        """生成PDF的主流程"""
        print("=" * 60)
        print(f"开始生成PDF文档 V2 - 语言: {language}")
        print("=" * 60)
        # 为每次生成创建独立的临时目录，避免跨语言复用导致清理困难
        try:
            import tempfile as _tempfile
            self.temp_dir = Path(_tempfile.mkdtemp())
            # 重置目录收集，避免多语言生成时相互污染
            self.toc_entries = []
            # 0. 加载项目信息（版本、版权等）
            self.project_meta = self._load_project_meta()
            # 1. 扫描文档结构
            print("1. 扫描文档结构...")
            documents = self.scanner.scan_documents(language)
            if not documents:
                print("✗ 未找到文档文件")
                return False
            
            total_docs = sum(len(docs) for docs in documents.values())
            print(f"✓ 找到 {total_docs} 个文档文件")
            
            # 准备资源输出目录（用于相对路径拷贝）
            self.assets_dir = self.temp_dir / 'assets'
            self.assets_dir.mkdir(exist_ok=True)

            # 记录章节结构来源（动态/硬编码）
            order = getattr(self.scanner, 'category_order', None)
            if order:
                print("✓ 章节结构: 动态 (来自 config.yaml:generation.output_structure)")
                print("  顺序: " + ", ".join(order))
            else:
                print("✓ 章节结构: 硬编码回退 (未在 config.yaml 中找到 output_structure)")

            # 2. 生成正文内容（先生成正文以便收集目录项）
            print("2. 生成正文内容...")
            content_html = self._generate_content(documents, language)

            # 3. 生成目录（依赖已收集的 toc_entries）
            print("3. 生成目录结构...")
            toc_html = self._generate_toc(documents, language)
            
            # 4. 创建完整的HTML文件
            print("4. 创建完整HTML文件...")
            full_html = self._create_full_html(title, toc_html, content_html, language)
            
            # 5. 生成PDF
            print("5. 生成PDF文件...")
            success = self._generate_pdf_from_html(full_html, title, language)
            
            if success:
                print("=" * 60)
                print("PDF生成完成!")
                print(f"📁 输出位置: {self.output_dir}")
                print("=" * 60)
                if getattr(self.scanner, 'category_order', None):
                    print("总结: 本次 PDF 章节名称与顺序根据 docs/source/config.yaml 动态生成")
                else:
                    print("总结: 本次 PDF 章节名称与顺序使用硬编码回退顺序")
                if getattr(self, 'keep_temp', False):
                    try:
                        print(f"临时目录保留: {self.temp_dir}")
                    except Exception:
                        pass
            
            return success
            
        except Exception as e:
            print(f"✗ PDF生成过程出错: {e}")
            return False
        finally:
            # 结束后清理本次生成的临时目录（除非要求保留）
            try:
                if not getattr(self, 'keep_temp', False) and getattr(self, 'temp_dir', None) and self.temp_dir.exists():
                    shutil.rmtree(self.temp_dir, ignore_errors=True)
            except Exception:
                pass
    
    def _generate_toc(self, documents: Dict[str, List[Dict]], language: str) -> str:
        """根据已收集的多级标题生成目录HTML（带点线引导）"""
        import re
        list_items = []
        for entry in self.toc_entries:
            indent = (entry['level'] - 1) * 18
            title_text = entry["title"]
            # 清洗标题中的前导列表符号或编号后的星号："* ", "- ", "• "、以及诸如 "1.2. * Title"
            title_text = re.sub(r'^[\s\*\-\u2022]+', '', title_text)
            title_text = re.sub(r'^(\d+(?:\.\d+)*)\.\s*[\*\-\u2022]+\s*', r'\1. ', title_text)
            # 再次去除可能残留的单独星号包围
            title_text = re.sub(r'\s+[\*\u2022]+\s+', ' ', title_text)
            list_items.append(
                f'<li class="toc-item level-{entry["level"]}" style="margin-left:{indent}px">'
                f'<a href="#{entry["anchor"]}"><span class="toc-text">{title_text}</span><span class="toc-dots"></span><span class="toc-page"></span></a>'
                f'</li>'
            )
        toc_title = '目录' if language == 'zh' else 'Contents'
        return (
            '<div class="toc">'
            f'<h2>{toc_title}</h2>'
            '<ul>' + ''.join(list_items) + '</ul>'
            '</div>'
        )
    
    def _generate_content(self, documents: Dict[str, List[Dict]], language: str) -> str:
        """生成正文内容HTML（分类、文档二级编号 + 文档内多级编号）"""
        content_sections = []
        
        category_index = 0
        # 按配置顺序渲染分类
        for category in getattr(self, 'category_order', []) or documents.keys():
            docs = documents.get(category, [])
            if not docs:
                continue
            
            # 本地化分类名
            # 根据语言选择分类名
            category_name = docs[0]['category_name'] if language == 'zh' else docs[0].get('category_name_en', docs[0]['category_name'])
            if language == 'en':
                cn_to_en = {
                    '快速上手': 'Getting Started',
                    '基础篇': 'Basics',
                    '驱动篇': 'Drivers',
                    '组件篇': 'Components',
                    '多媒体显示篇': 'Multimedia Display',
                    '多核通信篇': 'Multicore Communication',
                }
                category_name = cn_to_en.get(category_name, category_name)

            category_anchor = f'cat-{category}'
            category_index += 1
            category_number_text = f'{category_index}. '
            content_sections.append(f'<h1 id="{category_anchor}" class="category-title">{category_number_text}{category_name}</h1>')
            self.toc_entries.append({'level': 1, 'title': f'{category_number_text}{category_name}', 'anchor': category_anchor})
            
            doc_counter = 0
            for doc in docs:
                doc_id = f"doc-{self._slugify(doc['project_name'])}"
                content_sections.append(f'<section id="{doc_id}" class="document-section">')
                
                # 文档标题（二级）
                doc_counter += 1
                doc_anchor = f'{doc_id}-title'
                doc_number_text = f'{category_index}.{doc_counter}. '
                content_sections.append(f'<h2 id="{doc_anchor}" class="document-title">{doc_number_text}{doc["title"]}</h2>')
                self.toc_entries.append({'level': 2, 'title': f'{doc_number_text}{doc["title"]}', 'anchor': doc_anchor})
                
                # 文档内容与内部编号（三级及以下）
                doc_content = self._extract_document_content(doc, language)
                if doc_content:
                    numbered_html = self._auto_number_and_collect_toc(doc_content, base_numbers=[category_index, doc_counter], doc_title=doc["title"])
                    content_sections.append(numbered_html)
                else:
                    content_sections.append('<p>文档内容加载失败</p>')
                
                content_sections.append('</section>')
        
        return '\n'.join(content_sections)
    
    def _extract_document_content(self, doc_meta: Dict, language: str) -> str:
        """提取文档内容，并修复资源路径"""
        try:
            doc_file: Path = doc_meta['file']
            # 仅从Markdown文件提取内容
            with open(doc_file, 'r', encoding='utf-8') as f:
                md_content = f.read()
            
            # 预处理Markdown内容
            md_content = self._preprocess_markdown(md_content)
            
            # 处理Markdown
            html_content = self.md_processor.process_markdown(md_content)
            # 修复图片/链接等资源路径
            html_content = self._rewrite_resource_paths(html_content, doc_meta)
            return html_content
                
        except Exception as e:
            print(f"警告: 无法提取文档内容 {doc_file}: {e}")
            return ""
    
    def _text_to_html(self, text: str) -> str:
        """将纯文本转换为HTML格式"""
        if not text:
            return ""
        
        # 简单的文本到HTML转换
        lines = text.split('\n')
        html_lines = []
        
        for line in lines:
            line = line.strip()
            if not line:
                html_lines.append('<br>')
            elif line.startswith('#'):
                # 标题
                level = len(line) - len(line.lstrip('#'))
                title_text = line.lstrip('# ').strip()
                html_lines.append(f'<h{level}>{title_text}</h{level}>')
            elif line.startswith('- ') or line.startswith('* '):
                # 列表项
                html_lines.append(f'<li>{line[2:].strip()}</li>')
            elif line.startswith('```'):
                # 代码块
                html_lines.append('<pre><code>')
            elif line.endswith('```'):
                html_lines.append('</code></pre>')
            else:
                # 普通段落
                html_lines.append(f'<p>{line}</p>')
        
        return '\n'.join(html_lines)

    def _derive_projects_root(self) -> Path:
        """从 docs/source/config.yaml 读取项目根目录，找不到则回退到 ../../project"""
        try:
            cfg_path = Path(__file__).parent / 'config.yaml'
            if cfg_path.exists():
                import yaml
                with open(cfg_path, 'r', encoding='utf-8') as f:
                    cfg = yaml.safe_load(f) or {}
                    pdir = ((cfg.get('repository', {}) or {}).get('projects_dir', '../../project') or '../../project')
                    return (Path(__file__).parent / pdir).resolve()
        except Exception:
            pass
        return (Path(__file__).parent / '../../project').resolve()

    def _slugify(self, text: str) -> str:
        slug = re.sub(r'[^\w\-\.]+', '-', text, flags=re.UNICODE).strip('-').lower()
        return slug or 'section'

    def _rewrite_resource_paths(self, html: str, doc_meta: Dict) -> str:
        """将文档内相对资源路径改写为相对于合并HTML的相对路径。
        策略：把源图片复制到临时目录 temp/assets/<category>/<project>/，并将链接改为相对路径 assets/...。
        """
        try:
            soup = BeautifulSoup(html, 'html.parser')
            project_dir: Path = doc_meta.get('project_dir')
            category: str = doc_meta.get('category', '')
            project_name: str = doc_meta.get('project_name', '')
            docs_source_root = Path(__file__).parent
            # 目标相对目录（相对于合并HTML）
            rel_target_dir = Path('assets') / category / project_name
            abs_target_dir = (self.assets_dir or (self.temp_dir / 'assets')) / category / project_name
            abs_target_dir.mkdir(parents=True, exist_ok=True)

            def resolve_url(u: str) -> str:
                if not u:
                    return u
                s = u.strip()
                if s.startswith(('http://', 'https://', 'data:', 'mailto:', '#')):
                    return s
                # 尝试项目源码目录
                if project_dir is not None:
                    p1 = (project_dir / s)
                    try:
                        if p1.exists():
                            # 复制到目标 assets 目录并返回相对路径
                            dest = abs_target_dir / p1.name
                            try:
                                if p1.is_file():
                                    import shutil
                                    shutil.copy2(p1, dest)
                                return str((rel_target_dir / p1.name).as_posix())
                            except Exception:
                                pass
                    except Exception:
                        pass
                # 尝试 docs/source/<category>/<project_name>/
                p2 = (docs_source_root / category / project_name / s)
                try:
                    if p2.exists():
                        dest = abs_target_dir / p2.name
                        try:
                            if p2.is_file():
                                import shutil
                                shutil.copy2(p2, dest)
                            return str((rel_target_dir / p2.name).as_posix())
                        except Exception:
                            pass
                except Exception:
                    pass
                return s

            for tag in soup.select('img[src]'):
                tag['src'] = resolve_url(tag.get('src'))
            for tag in soup.select('a[href]'):
                href = tag.get('href')
                if href and not href.startswith('#'):
                    tag['href'] = resolve_url(href)
            return str(soup)
        except Exception:
            return html

    def _auto_number_and_collect_toc(self, html: str, base_numbers: List[int], doc_title: Optional[str] = None) -> str:
        """为文档内部标题自动编号并收集目录（最大深度到第3级）。
        规则：目录深度仅到 3 级：分类(1) -> 文档(2) -> 文档内首级标题(3)。
        base_numbers: 分类号与文档号作为前缀，例如 [2,1] -> 2.1.x
        """
        try:
            soup = BeautifulSoup(html, 'html.parser')
            # 避免 Sphinx headerlink 图标导致的方框，移除
            for node in soup.select('a.headerlink'):
                node.decompose()

            # 规范化：如果文档首个标题不是 h1，则将其作为 h1 起始
            headers = soup.find_all([f'h{i}' for i in range(1, 4)])
            first_level = None
            for h in headers:
                first_level = int(h.name[1])
                break
            if first_level and first_level > 1:
                for h in headers:
                    lvl = int(h.name[1])
                    h.name = f'h{max(1, lvl - first_level + 1)}'

            # 如果文档首个 h1 与外层文档标题相同，则删除该 h1，避免目录重复（例如 1.1 与 1.1.1 重复）
            if doc_title:
                # 取第一个 h1
                first_h1 = soup.find('h1')
                if first_h1:
                    def _normalize(text: str) -> str:
                        import re as _re
                        t = (text or '').strip()
                        t = _re.sub(r'[\s\*\u2022]+', ' ', t)
                        t = _re.sub(r'\s+', ' ', t)
                        return t
                    if _normalize(first_h1.get_text()) == _normalize(doc_title):
                        # 删除该 h1，不计入目录
                        first_h1.decompose()

            # 仅对文档内首级标题(h1)编号，保证总深度不超过 3
            local_counter = 0
            for header in soup.find_all('h1'):
                local_counter += 1
                parts = [str(n) for n in base_numbers] + [str(local_counter)]
                number_text = '.'.join(parts) + '. '

                # 原标题文本
                original_text = header.get_text(strip=True)
                # 写回：前置编号
                # 清空子节点，仅保留纯文本（保留强调等会复杂，这里简化）
                header.clear()
                header.string = number_text + original_text

                # 设置锚点 id
                anchor = header.get('id') or self._slugify(original_text)
                header['id'] = anchor

                # 收集目录项：层级等于数字段数
                toc_level = len(parts)
                self.toc_entries.append({'level': toc_level, 'title': number_text + original_text, 'anchor': anchor})

            return str(soup)
        except Exception:
            return html
    
    def _preprocess_markdown(self, md_content: str) -> str:
        """预处理Markdown内容"""
        if not md_content:
            return ""
        
        lines = md_content.split('\n')
        processed_lines = []
        in_code_block = False
        code_block_language = ""
        
        for line in lines:
            # 处理代码块
            if line.strip().startswith('```'):
                if not in_code_block:
                    # 开始代码块
                    in_code_block = True
                    code_block_language = line.strip()[3:].strip()
                    processed_lines.append(f'```{code_block_language}')
                else:
                    # 结束代码块
                    in_code_block = False
                    code_block_language = ""
                    processed_lines.append('```')
                continue
            
            # 在代码块内，保持原样
            if in_code_block:
                processed_lines.append(line)
                continue
            
            # 处理标题
            if line.strip().startswith('#'):
                # 确保标题前后有空行
                if processed_lines and processed_lines[-1].strip():
                    processed_lines.append('')
                processed_lines.append(line)
                processed_lines.append('')
                continue
            
            # 处理列表项
            if line.strip().startswith(('- ', '* ', '+ ')):
                # 确保列表前有空行
                if processed_lines and processed_lines[-1].strip() and not processed_lines[-1].strip().startswith(('- ', '* ', '+ ')):
                    processed_lines.append('')
                processed_lines.append(line)
                continue
            
            # 处理数字列表
            if re.match(r'^\s*\d+\.\s+', line):
                # 确保列表前有空行
                if processed_lines and processed_lines[-1].strip() and not re.match(r'^\s*\d+\.\s+', processed_lines[-1]):
                    processed_lines.append('')
                processed_lines.append(line)
                continue
            
            # 处理表格
            if '|' in line and line.strip():
                # 确保表格前有空行
                if processed_lines and processed_lines[-1].strip() and '|' not in processed_lines[-1]:
                    processed_lines.append('')
                processed_lines.append(line)
                continue
            
            # 处理空行
            if not line.strip():
                # 避免连续的空行
                if processed_lines and processed_lines[-1].strip():
                    processed_lines.append('')
                continue
            
            # 处理普通段落
            processed_lines.append(line)
        
        # 清理末尾的空行
        while processed_lines and not processed_lines[-1].strip():
            processed_lines.pop()
        
        return '\n'.join(processed_lines)
    
    def _create_full_html(self, title: str, toc_html: str, content_html: str, language: str) -> Path:
        """创建完整的HTML文件"""
        html_file = self.temp_dir / f"merged_{language}.html"
        
        # 根据语言设置字体
        if language == 'zh':
            # 扩充中文字体回退链，兼容 Linux CI（Noto/WenQuanYi/Droid）
            font_family = (
                '"Microsoft YaHei", "SimSun", '
                '"Noto Sans CJK SC", "Noto Sans CJK", "Noto Sans SC", '
                '"WenQuanYi Zen Hei", "Droid Sans Fallback", sans-serif'
            )
            lang_attr = "zh-CN"
        else:
            font_family = '"Arial", "Helvetica", sans-serif'
            lang_attr = "en"
        
        cover_subtitle = ('开发文档' if language == 'zh' else 'Documentation')
        label_version = ('版本' if language == 'zh' else 'Version')
        label_language = ('语言' if language == 'zh' else 'Language')
        label_generated = ('生成时间' if language == 'zh' else 'Generated on')

        # 封面信息（徽章 + 详情行）
        badge_lang = ('中文' if language == 'zh' else 'English')
        badge_version = (self.project_meta.get('version') or '1.0.0')
        badge_date = datetime.now().strftime('%Y年%m月%d日' if language == 'zh' else '%B %d, %Y')
        meta_badges_html = (
            '<div class="meta-badges">'
            f'<span class="pill">Version {badge_version}</span>'
            '<span class="sep">|</span>'
            f'<span class="pill">{badge_lang}</span>'
            '<span class="sep">|</span>'
            f'<span class="pill">{badge_date}</span>'
            '</div>'
        )

        detail_items = []
        author = (self.project_meta.get('author') or '').strip()
        website = (self.project_meta.get('website') or '').strip()
        copyright_txt = (self.project_meta.get('copyright') or '').strip()
        if author:
            detail_items.append(( '作者' if language == 'zh' else 'Author', author ))
        if website:
            website_display = website.replace('官网: ', '').replace('Website: ', '')
            detail_items.append(('', website_display))  # 空键名，不显示标签
        if copyright_txt:
            copyright_display = copyright_txt.replace('版权: ', '').replace('Copyright: ', '')
            detail_items.append(('', copyright_display))  # 空键名，不显示标签
        if detail_items:
            lines = []
            for key, val in detail_items:
                if not key:  # 如果键名为空，只显示值
                    if val.startswith(('http://', 'https://')):
                        lines.append(f'<div class="meta-line"><a href="{val}">{val}</a></div>')
                    else:
                        lines.append(f'<div class="meta-line">{val}</div>')
                else:
                    if key.lower().startswith(('官网','website')) and (val.startswith('http://') or val.startswith('https://')):
                        lines.append(f'<div class="meta-line"><span class="k">{key}:</span> <a href="{val}">{val}</a></div>')
                    else:
                        lines.append(f'<div class="meta-line"><span class="k">{key}:</span> {val}</div>')
            meta_details_html = '<div class="meta-details">' + ''.join(lines) + '</div>'
        else:
            meta_details_html = ''

        html_template = f'''<!DOCTYPE html>
<html lang="{lang_attr}">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{title}</title>
    <style>
        @page {{
            size: A4;
            margin-top: 1.50cm;
            margin-bottom: 1.75cm;
            margin-left: 1.10cm;
            margin-right: 1.10cm;
        }}
        
        body {{
            font-family: {font_family};
            line-height: 1.5;
            margin: 0;
            padding: 0;
            color: #222;
            font-size: 12pt;
        }}
        
        /* 封面样式 */
        .cover-page {{
            /* 避免封面后出现空白页 */
            page-break-after: avoid;
            text-align: center;
            padding: 2.2cm 2cm 1.2cm 2cm;
            height: auto;
            min-height: calc(100vh - 3.0cm);
            display: flex;
            flex-direction: column;
            justify-content: flex-start;
            align-items: center;
            margin-bottom: 0.8cm;
        }}
        
        .cover-title {{
            font-size: 2.5em;
            font-weight: bold;
            color: #2c3e50;
            margin-bottom: 1em;
            border-bottom: 3px solid #3498db;
            padding-bottom: 0.5em;
        }}
        
        .cover-subtitle {{
            font-size: 1.2em;
            color: #7f8c8d;
            margin-top: 0.6em;
            margin-bottom: 1.6em;
        }}

        .cover-footer {{
            position: absolute;
            left: 0;
            right: 0;
            bottom: 1.8cm;
            text-align: center;
        }}
        .meta-badges {{
            display: inline-flex;
            align-items: center;
            gap: 10px;
            margin-bottom: 0.6em;
        }}
        .meta-badges .pill {{
            display: inline-block;
            padding: 4px 10px;
            border-radius: 9999px;
            font-size: 0.9em;
            color: #2c3e50;
            background: #f3f6fa;
            border: 1px solid #dde5ee;
        }}
        .meta-badges .sep {{
            color: #95a5a6;
            font-size: 0.95em;
        }}
        .meta-details {{
            font-size: 0.95em;
            color: #6b7280;
        }}
        .meta-details .meta-line {{ margin: 4px 0; }}
        .meta-details .k {{
            color: #374151;
            font-weight: 600;
        }}
        .meta-details a {{
            color: #2563eb;
            text-decoration: none;
        }}
        .meta-details a:hover {{
            text-decoration: underline;
        }}
        
        .cover-info {{
            font-size: 1em;
            color: #95a5a6;
            margin-top: auto;
        }}
        
        /* 目录样式 */
        .toc {{
            page-break-after: always;
            padding: 1.3cm 2cm 1.6cm 2cm;
        }}
        
        .toc h2 {{
            font-size: 1.8em;
            color: #2c3e50;
            text-align: left;
            margin: 0 0 0.8em 0;
            border-left: 4px solid #3498db;
            padding-left: 0.6em;
        }}
        
        .toc ul {{
            list-style: none;
            padding: 0;
            margin: 0;
            max-width: 16cm;
        }}
        
        .toc-category {{
            margin: 0.6em 0 0.35em 0;
            font-size: 1.04em;
            color: #334155;
            border-bottom: 1px solid #e2e8f0;
            padding-bottom: 0.25em;
            letter-spacing: 0.2px;
        }}
        
        .toc-item {{
            margin: 0.12em 0;
            padding: 0.12em 0;
            line-height: 1.38;
        }}
        
        .toc a {{
            text-decoration: none;
            color: #2c3e50;
            font-size: 0.95em;
            display: block;
            padding: 0.1em 0;
        }}
        .toc a {{ display: grid; grid-template-columns: auto 1fr auto; align-items: baseline; column-gap: 8px; }}
        .toc a .toc-text {{ white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }}
        .toc a .toc-dots {{ border-bottom: 1px dotted #cbd5e1; height: 0; margin-top: 0.55em; }}
        .toc a .toc-page {{ color: #475569; font-variant-numeric: tabular-nums; padding-left: 4px; }}
        
        .toc a:hover {{
            color: #2563eb;
            background: #f8fafc;
            padding-left: 0.4em;
            transition: all 0.2s ease;
        }}
        
        /* 正文样式 */
        .content {{
            padding: 2cm;
        }}
        /* 脚注样式 */
        .footnote-ref {{
            vertical-align: super;
            font-size: 0.8em;
        }}
        .footnotes {{
            font-size: 0.9em;
            color: #555;
        }}
        .footnotes hr {{
            border: none;
            border-top: 1px solid #ddd;
            margin: 1em 0;
        }}
        .footnotes ol {{
            padding-left: 1.2em;
        }}
        .footnotes li {{
            margin: 0.4em 0;
        }}
        
        .category-title {{
            color: #2c3e50;
            border-bottom: 2px solid #3498db;
            padding-bottom: 10px;
            margin-top: 2em;
            margin-bottom: 1em;
            font-size: 1.8em;
            page-break-before: always;
        }}
        
        .category-title:first-child {{
            page-break-before: auto;
        }}
        
        .document-section {{
            margin-bottom: 1.2em;
            page-break-inside: avoid;
        }}
        
        .document-title {{
            color: #2c3e50;
            border-bottom: 1px solid #bdc3c7;
            padding-bottom: 8px;
            margin-top: 1.5em;
            margin-bottom: 1em;
            font-size: 1.4em;
        }}
        
        h1, h2, h3, h4, h5, h6 {{
            color: #2c3e50;
            margin-top: 1.5em;
            margin-bottom: 0.8em;
        }}
        
        h3 {{
            font-size: 1.2em;
            color: #7f8c8d;
        }}
        
        p {{
            margin: 0.6em 0;
            text-align: justify;
        }}
        
        code {{
            background: #f8f9fa;
            padding: 2px 6px;
            border-radius: 3px;
            font-family: "Consolas", "Monaco", monospace;
            border: 1px solid #e9ecef;
            font-size: 0.9em;
            color: #e83e8c;
        }}
        
        pre {{
            background: #f8f9fa;
            padding: 0.8em;
            border-radius: 5px;
            overflow-x: auto;
            margin: 0.6em 0;
            border: 1px solid #e9ecef;
            font-family: "Consolas", "Monaco", monospace;
            font-size: 0.9em;
            line-height: 1.4;
        }}
        
        pre code {{
            background: none;
            padding: 0;
            border: none;
            color: #333;
            font-size: inherit;
        }}
        
        ul, ol {{
            margin: 0.6em 0;
            padding-left: 2em;
        }}
        
        li {{
            margin: 0.5em 0;
            line-height: 1.6;
        }}
        
        ul ul, ol ol, ul ol, ol ul {{
            margin: 0.5em 0;
        }}
        
        /* 改进列表样式 */
        ul li {{
            list-style-type: disc;
        }}
        
        ul ul li {{
            list-style-type: circle;
        }}
        
        ul ul ul li {{
            list-style-type: square;
        }}
        
        ol li {{
            list-style-type: decimal;
        }}
        
        ol ol li {{
            list-style-type: lower-alpha;
        }}
        
        ol ol ol li {{
            list-style-type: lower-roman;
        }}
        
        img {{
            max-width: 100%;
            height: auto;
            margin: 1em 0;
            border: 1px solid #ddd;
            border-radius: 3px;
        }}
        
        table {{
            border-collapse: collapse;
            width: 100%;
            margin: 1.5em 0;
            border: 1px solid #ddd;
        }}
        
        th, td {{
            border: 1px solid #ddd;
            padding: 0.8em 1em;
            text-align: left;
        }}
        
        th {{
            background: #f8f9fa;
            font-weight: bold;
            color: #2c3e50;
        }}
        
        blockquote {{
            border-left: 4px solid #e74c3c;
            margin: 1.5em 0;
            padding: 1em 1.5em;
            background: #fdf2f2;
            font-style: italic;
        }}
        
        /* 改进表格样式 */
        table {{
            border-collapse: collapse;
            width: 100%;
            margin: 1.5em 0;
            border: 1px solid #ddd;
            font-size: 0.9em;
        }}
        
        th, td {{
            border: 1px solid #ddd;
            padding: 0.8em 1em;
            text-align: left;
            vertical-align: top;
        }}
        
        th {{
            background: #f8f9fa;
            font-weight: bold;
            color: #2c3e50;
        }}
        
        tr:nth-child(even) {{
            background: #f9f9f9;
        }}
        
        /* 改进链接样式 */
        a {{
            color: #3498db;
            text-decoration: none;
        }}
        
        a:hover {{
            text-decoration: underline;
        }}
        
        /* 改进强调样式 */
        strong, b {{
            font-weight: bold;
            color: #2c3e50;
        }}
        
        em, i {{
            font-style: italic;
            color: #7f8c8d;
        }}
        
        /* 改进水平线样式 */
        hr {{
            border: none;
            border-top: 2px solid #bdc3c7;
            margin: 2em 0;
        }}
        
        /* 改进引用块样式 */
        .highlight {{
            background: #f8f9fa;
            border: 1px solid #e9ecef;
            border-radius: 5px;
            padding: 1em;
            margin: 1em 0;
        }}
        
        /* 改进警告框样式 */
        .admonition {{
            border: 1px solid #ddd;
            border-radius: 5px;
            padding: 1em;
            margin: 1em 0;
            background: #f9f9f9;
        }}
        
        .admonition-title {{
            font-weight: bold;
            margin-bottom: 0.5em;
            color: #2c3e50;
        }}
        
        /* 打印样式 */
        @media print {{
            .cover-page {{
                page-break-after: avoid;
            }}
            
            .toc {{
                page-break-after: always;
            }}
            
            .category-title {{
                page-break-before: always;
            }}
            
            .document-section {{
                page-break-inside: avoid;
            }}
            
            h1, h2, h3 {{
                page-break-after: avoid;
            }}
            
            body {{
                margin: 0;
                padding: 0;
            }}
        }}
    </style>
</head>
<body>
    
    <!-- 封面页 -->
    <div class="cover-page">
        <div class="cover-title">{title}</div>
        <div class="cover-subtitle">{cover_subtitle}</div>
        <div class="cover-footer">
            {meta_badges_html}
            {meta_details_html}
        </div>
    </div>
    
    
    <!-- 目录页 -->
    {toc_html}
    
    <!-- 正文内容 -->
    <div class="content">
        {content_html}
    </div>
</body>
</html>'''
        
        with open(html_file, 'w', encoding='utf-8') as f:
            f.write(html_template)
        
        print(f"✓ 已创建完整HTML文件: {html_file}")
        return html_file

    def _load_project_meta(self) -> Dict[str, str]:
        """从 docs/source/config.yaml 读取项目信息（名称、版本、版权等）"""
        meta = {"name": "", "version": "", "copyright": "", "website": ""}
        try:
            cfg_path = Path(__file__).parent / 'config.yaml'
            if cfg_path.exists():
                import yaml
                with open(cfg_path, 'r', encoding='utf-8') as f:
                    cfg = yaml.safe_load(f) or {}
                    proj = (cfg.get('project') or {})
                    meta["name"] = proj.get('name', '')
                    meta["version"] = proj.get('version', '')
                    meta["website"] = proj.get('website', '')
                    meta["copyright"] = (proj.get('copyright')
                        or (cfg.get('project', {}).get('copyright'))
                        or '')
        except Exception:
            pass
        return meta
    
    def _generate_pdf_from_html(self, html_file: Path, title: str, language: str) -> bool:
        """从HTML文件生成PDF"""
        try:
            # 确保输出目录存在
            self.output_dir.mkdir(parents=True, exist_ok=True)
            
            # 生成PDF文件名
            if language == 'zh':
                pdf_filename = f"{title}.pdf"
            else:
                # 英文版：将空格替换为下划线，再追加 _EN
                safe_title = (title or '').replace(' ', '_')
                pdf_filename = f"{safe_title}_EN.pdf"
            
            output_pdf = self.output_dir / pdf_filename
            
            # 尝试使用Chrome/Edge的headless模式生成PDF
            if self._try_chrome_pdf(html_file, output_pdf):
                print(f"✓ PDF生成成功: {output_pdf}")
                return True
            
            # 回退到手动方式
            print("⚠️  自动PDF生成失败，回退到手动方式...")
            return self._generate_pdf_manual(html_file, output_pdf)
            
        except Exception as e:
            print(f"✗ PDF生成失败: {e}")
            return False
    
    def _try_chrome_pdf(self, html_file: Path, output_pdf: Path) -> bool:
        """尝试使用Chrome生成PDF"""
        try:
            # 优先使用用户提供的浏览器路径
            if getattr(self, 'browser_path', None):
                if os.path.exists(self.browser_path):
                    browser_cmd = self.browser_path
                else:
                    browser_cmd = None
            else:
                browser_cmd = None

            # 优先使用环境变量指定的浏览器路径
            env_browser = os.environ.get('CHROME_PATH') or os.environ.get('BROWSER')
            browser_paths = [
                r"C:\\Program Files\\Google\\Chrome\\Application\\chrome.exe",
                r"C:\\Program Files (x86)\\Google\\Chrome\\Application\\chrome.exe",
                r"C:\\Program Files\\Microsoft\\Edge\\Application\\msedge.exe",
                r"C:\\Program Files (x86)\\Microsoft\\Edge\\Application\\msedge.exe",
            ]
            if env_browser:
                browser_paths.insert(0, env_browser)
            # 也尝试通过 PATH 查找
            try:
                import shutil as _shutil
                for candidate in [
                    "chrome.exe", "msedge.exe", "chrome", "msedge",
                    "google-chrome", "google-chrome-stable", "chromium", "chromium-browser"
                ]:
                    p = _shutil.which(candidate)
                    if p and os.path.exists(p):
                        browser_paths.insert(0, p)
                        break
            except Exception:
                pass
            
            if not browser_cmd:
                for path in browser_paths:
                    if os.path.exists(path):
                        browser_cmd = path
                        break
            
            if not browser_cmd:
                return False
            
            # 构建命令
            cmd = [
                browser_cmd,
                "--headless=new",
                "--disable-gpu",
                "--no-sandbox",
                "--disable-dev-shm-usage",
                "--no-first-run",
                "--disable-extensions",
                "--disable-background-networking",
                "--disable-default-apps",
                "--disable-sync",
                "--disable-translate",
                "--disable-features=PrintingPdfHeaderFooter,TranslateUI",
                f"--print-to-pdf={str(output_pdf.resolve().as_posix())}",
                "--print-to-pdf-no-header",
                "--no-pdf-header-footer",
                "--disable-print-preview",
                "--run-all-compositor-stages-before-draw",
                "--virtual-time-budget=20000",
                f"file:///{html_file.resolve().as_posix()}"
            ]
            
            # 执行命令（以二进制模式捕获输出，避免控制台编码问题）
            result = subprocess.run(cmd, capture_output=True, text=False, timeout=60)

            ok = result.returncode == 0 and output_pdf.exists()
            if not ok:
                try:
                    stderr_text = (result.stderr or b"").decode(errors="ignore")
                    stdout_text = (result.stdout or b"").decode(errors="ignore")
                    print("Chrome/Edge 无头打印失败: ")
                    if stdout_text:
                        print(stdout_text[:800])
                    if stderr_text:
                        print(stderr_text[:800])
                except Exception:
                    pass
            return ok
            
        except Exception as e:
            print(f"Chrome PDF生成失败: {e}")
            return False
    
    def _generate_pdf_manual(self, html_file: Path, output_pdf: Path) -> bool:
        """手动生成PDF（回退方案）"""
        try:
            # 打开浏览器
            import webbrowser
            url = f"file:///{html_file.absolute()}"
            webbrowser.open(url)
            
            print("\n" + "="*60)
            print("📄 PDF生成说明:")
            print("1. 浏览器已自动打开HTML页面")
            print("2. 使用浏览器打印（Ctrl+P），目标选择“保存为 PDF”")
            print("3. 重要：取消勾选“页眉和页脚”（Headers and footers）以移除浏览器默认的时间、URL、页码")
            print("4. 如需保持版式，建议边距使用默认或无")
            print(f"5. 保存到: {output_pdf}")
            print("6. 完成后按回车键继续...")
            print("="*60)
            
            try:
                input("按回车键继续...")
            except EOFError:
                # 非交互环境下没有标准输入，直接继续
                pass
            return True
            
        except Exception as e:
            print(f"手动PDF生成失败: {e}")
            return False

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="增强版PDF生成器V2")
    parser.add_argument('--html-dir', type=str, default='source_build/html/latest',
                       help='HTML文件目录 (默认: source_build/html/latest)')
    parser.add_argument('--output-dir', type=str, default='source_build/html/latest/_static',
                       help='PDF输出目录 (默认: source_build/html/latest/_static)')
    parser.add_argument('--title', type=str, default='Titan-Board SDK文档',
                       help='PDF标题 (默认: Titan-Board SDK文档)')
    parser.add_argument('--language', type=str, default='zh',
                       choices=['zh', 'en'],
                       help='文档语言 (默认: zh)')
    parser.add_argument('--both', action='store_true',
                       help='同时生成中英文版本')
    parser.add_argument('--keep-temp', action='store_true',
                       help='保留临时目录以便调试（输出merged_*.html路径）')
    parser.add_argument('--browser', type=str, default='',
                       help='指定 Chrome/Edge 浏览器可执行文件路径')
    
    args = parser.parse_args()
    
    # 转换为Path对象
    html_dir = Path(args.html_dir)
    output_dir = Path(args.output_dir)
    
    if not html_dir.exists():
        print(f"✗ HTML目录不存在: {html_dir}")
        sys.exit(1)
    
    # 创建PDF生成器
    generator = PDFGeneratorV2(html_dir, output_dir, keep_temp=args.keep_temp, browser_path=(args.browser or None))
    
    if args.both:
        # 生成中英文两个版本
        success_zh = generator.generate_pdf(args.title, "zh")
        success_en = generator.generate_pdf(args.title, "en")
        success = success_zh and success_en
    else:
        # 生成指定语言版本
        success = generator.generate_pdf(args.title, args.language)
    
    sys.exit(0 if success else 1)

if __name__ == "__main__":
    main()