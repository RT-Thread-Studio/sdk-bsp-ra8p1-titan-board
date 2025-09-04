#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
中央构建管理器
完全基于 .github/versions.json 动态生成分支名称和构建配置
支持 Git Worktree 隔离构建，避免分支切换问题
"""

import os
import sys
import json
import shutil
import subprocess
import argparse
import platform
from pathlib import Path
from typing import List, Dict, Optional, Union
from shutil import which
import yaml

class VersionConfig:
    """版本配置类"""
    def __init__(self, config_dict: Dict):
        self.name = config_dict['name']
        self.display_name = config_dict['display_name']
        self.branch = config_dict['branch']
        self.url_path = config_dict['url_path']
        self.description = config_dict.get('description', '')

class BuildManager:
    """构建管理器"""
    
    def __init__(self):
        self.project_root = self._find_project_root()
        self.versions_file = self.project_root / '.github' / 'versions.json'
        self.docs_source = self.project_root / 'docs' / 'source'
        # 统一切换到新的构建输出根目录: source_build/html/<version>
        self.build_root = self.docs_source / 'source_build'
        self.worktrees_dir = self.build_root / 'worktrees'
        self.versions_dir = self.build_root / 'html'
        
    def _find_project_root(self) -> Path:
        """查找项目根目录"""
        current = Path.cwd()
        while current != current.parent:
            if (current / '.github' / 'versions.json').exists():
                return current
            current = current.parent
        raise FileNotFoundError("找不到 .github/versions.json 文件")
    
    def load_versions_config(self) -> Dict:
        """加载版本配置文件"""
        try:
            with open(self.versions_file, 'r', encoding='utf-8') as f:
                config = json.load(f)
            print(f"✓ 加载版本配置: {[v['name'] for v in config.get('versions', [])]}")
            return config
        except Exception as e:
            print(f"✗ 无法加载版本配置: {e}")
            return {'versions': [], 'default_version': '', 'latest_version': ''}
    
    def get_version_configs(self) -> List[VersionConfig]:
        """获取版本配置列表"""
        config = self.load_versions_config()
        versions = []
        for version_dict in config.get('versions', []):
            versions.append(VersionConfig(version_dict))
        return versions
    
    def create_worktree(self, version_config: VersionConfig) -> Path:
        """为指定版本创建 Git worktree"""
        worktree_path = self.worktrees_dir / version_config.name
        
        # 获取当前分支
        current_branch = subprocess.run(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            capture_output=True, text=True, check=True
        ).stdout.strip()
        
        # 如果目标分支就是当前分支，直接使用当前目录
        if version_config.branch == current_branch:
            print(f"目标分支 {version_config.branch} 就是当前分支，使用当前目录")
            return Path.cwd()
        
        # 清理已存在的 worktree
        if worktree_path.exists():
            print(f"清理已存在的 worktree: {worktree_path}")
            try:
                subprocess.run(['git', 'worktree', 'remove', str(worktree_path)], 
                             check=True, capture_output=True)
            except subprocess.CalledProcessError:
                # 如果 worktree remove 失败，手动删除
                shutil.rmtree(worktree_path, ignore_errors=True)
        
        # 创建新的 worktree
        print(f"创建 worktree: {version_config.branch} -> {worktree_path}")
        subprocess.run([
            'git', 'worktree', 'add', 
            str(worktree_path), version_config.branch
        ], check=True)
        
        return worktree_path
    
    def build_docs_in_worktree(self, worktree_path: Path, version_config: VersionConfig) -> bool:
        """在 worktree 中构建文档"""
        print(f"在 worktree 中构建文档: {worktree_path}")
        
        # 检查 docs/source 目录是否存在
        if worktree_path == Path.cwd():
            # 如果是当前分支，直接使用当前目录
            docs_source_in_worktree = Path.cwd()
        else:
            docs_source_in_worktree = worktree_path / 'docs' / 'source'
            if not docs_source_in_worktree.exists():
                print(f"⚠️  警告: {worktree_path} 中没有 docs/source 目录")
                print(f"   使用主分支的文档结构进行构建")
                # 复制主分支的文档结构
                main_docs = self.docs_source
                if main_docs.exists():
                    shutil.copytree(main_docs, docs_source_in_worktree, dirs_exist_ok=True)
                else:
                    print(f"✗ 错误: 主分支也没有 docs/source 目录")
                    return False
        
        # 切换到 worktree 目录（如果不是当前分支）
        if worktree_path != Path.cwd():
            os.chdir(worktree_path)
        
        try:
            # 读取项目名称用于 PDF 命名
            project_name = 'SDK_Docs'
            try:
                cfg_path = docs_source_in_worktree / 'config.yaml'
                if cfg_path.exists():
                    with open(cfg_path, 'r', encoding='utf-8') as f:
                        cfg = yaml.safe_load(f) or {}
                        project_name = (cfg.get('project', {}) or {}).get('name', project_name)
            except Exception:
                pass
            def _slugify(name: str) -> str:
                safe = []
                for ch in name:
                    if ch.isalnum() or ('\u4e00' <= ch <= '\u9fa5'):
                        safe.append(ch)
                    elif ch in [' ', '-', '_']:
                        safe.append('_' if ch == ' ' else ch)
                s = ''.join(safe).strip('_')
                return s or 'SDK_Docs'
            pdf_basename = _slugify(project_name) + '.pdf'
            # 运行文档生成脚本（如果存在）
            doc_generator = docs_source_in_worktree / 'doc_generator.py'
            if doc_generator.exists():
                print(f"运行文档生成脚本: {doc_generator}")
                subprocess.run([sys.executable, str(doc_generator)], 
                             cwd=str(docs_source_in_worktree), check=True)
            
            # 嵌入版本配置
            embed_script = docs_source_in_worktree / 'utils' / 'embed_version_config.py'
            if embed_script.exists():
                print(f"嵌入版本配置: {embed_script}")
                subprocess.run([sys.executable, str(embed_script)], 
                             cwd=str(docs_source_in_worktree), check=True)
            
            # 构建 HTML 文档
            # 输出到新的路径: source_build/html/<version_url_path>
            output_dir = self.build_root / 'html' / version_config.url_path
            print(f"构建 HTML 文档: {output_dir}")
            subprocess.run([
                sys.executable, '-m', 'sphinx.cmd.build',
                '-b', 'html',
                str(docs_source_in_worktree),
                str(output_dir)
            ], check=True)
            
            # 生成版本配置（注入项目源目录片段与复制文件规则）
            # 从 docs/source/config.yaml 读取 repository.projects_dir，并转换为仓库内相对路径片段
            projects_dir_web = ''
            copy_files_list = []
            try:
                cfg_path = docs_source_in_worktree / 'config.yaml'
                if cfg_path.exists():
                    with open(cfg_path, 'r', encoding='utf-8') as f:
                        repo_cfg = yaml.safe_load(f) or {}
                        pdir = ((repo_cfg.get('repository', {}) or {}).get('projects_dir', '') or '').replace('\\','/')
                        # 若是相对路径如 ../../project，则仅取末段 "project"
                        if pdir:
                            parts = [seg for seg in pdir.split('/') if seg and seg != '..' and seg != '.']
                            if parts:
                                projects_dir_web = '/'.join(parts[-1:])
                        copy_files_list = ((repo_cfg.get('generation', {}) or {}).get('copy_files', []) or [])
            except Exception:
                pass

            self._generate_version_config(output_dir, version_config, projects_dir_web, copy_files_list)

            # 构建 PDF（优先使用Windows专用生成器）
            pdf_file = None
            try:
                print("尝试使用Windows专用PDF生成器...")
                from pdf_generator_windows import WindowsPDFGenerator
                
                # 创建PDF生成器
                pdf_generator = WindowsPDFGenerator(output_dir, output_dir / '_static')
                
                # 生成PDF
                if pdf_generator.generate(project_name):
                    # 查找生成的PDF文件
                    static_dir = output_dir / '_static'
                    candidate_pdf = static_dir / 'sdk-docs.pdf'
                    if candidate_pdf.exists():
                        pdf_file = candidate_pdf
                        print(f"✓ 使用Windows专用生成器生成PDF成功: {pdf_file}")
                    else:
                        print("⚠️  PDF文件未找到，请手动保存PDF文件")
                else:
                    print("⚠️  Windows专用PDF生成失败，尝试增强版...")
                    # 回退到增强版
                    from pdf_generator_enhanced import EnhancedPDFGenerator
                    pdf_generator = EnhancedPDFGenerator(output_dir, output_dir / '_static')
                    if pdf_generator.generate(project_name, method="auto"):
                        static_dir = output_dir / '_static'
                        candidate_pdf = static_dir / 'sdk-docs.pdf'
                        if candidate_pdf.exists():
                            pdf_file = candidate_pdf
                            print(f"✓ 使用增强版生成器生成PDF成功: {pdf_file}")
                    
            except ImportError:
                print("⚠️  Windows专用PDF生成器模块未找到，尝试增强版...")
                try:
                    from pdf_generator_enhanced import EnhancedPDFGenerator
                    pdf_generator = EnhancedPDFGenerator(output_dir, output_dir / '_static')
                    if pdf_generator.generate(project_name, method="auto"):
                        static_dir = output_dir / '_static'
                        candidate_pdf = static_dir / 'sdk-docs.pdf'
                        if candidate_pdf.exists():
                            pdf_file = candidate_pdf
                            print(f"✓ 使用增强版生成器生成PDF成功: {pdf_file}")
                except ImportError:
                    print("⚠️  增强版PDF生成器模块也未找到，尝试简化版...")
                    try:
                        from pdf_generator_simple import SimplePDFGenerator
                        pdf_generator = SimplePDFGenerator(output_dir, output_dir / '_static')
                        if pdf_generator.generate(project_name):
                            static_dir = output_dir / '_static'
                            candidate_pdf = static_dir / 'sdk-docs.pdf'
                            if candidate_pdf.exists():
                                pdf_file = candidate_pdf
                                print(f"✓ 使用简化版生成器生成PDF成功: {pdf_file}")
                    except ImportError:
                        print("⚠️  所有PDF生成器模块都未找到，尝试传统LaTeX方法...")
                        pdf_file = self._generate_pdf_latex(docs_source_in_worktree, version_config)
            except Exception as e:
                print(f"⚠️  Windows专用PDF生成出错: {e}，尝试增强版...")
                try:
                    from pdf_generator_enhanced import EnhancedPDFGenerator
                    pdf_generator = EnhancedPDFGenerator(output_dir, output_dir / '_static')
                    if pdf_generator.generate(project_name, method="auto"):
                        static_dir = output_dir / '_static'
                        candidate_pdf = static_dir / 'sdk-docs.pdf'
                        if candidate_pdf.exists():
                            pdf_file = candidate_pdf
                            print(f"✓ 使用增强版生成器生成PDF成功: {pdf_file}")
                except Exception as e2:
                    print(f"⚠️  增强版PDF生成也出错: {e2}，尝试传统LaTeX方法...")
                    pdf_file = self._generate_pdf_latex(docs_source_in_worktree, version_config)

            # 将 PDF 复制到 HTML 的 _static 目录，供在线下载
            if pdf_file and pdf_file.exists():
                static_dir = output_dir / '_static'
                static_dir.mkdir(exist_ok=True)
                target_pdf = static_dir / pdf_basename
                shutil.copy2(pdf_file, target_pdf)
                print(f"✓ 生成并复制 PDF: {pdf_file.name} -> {target_pdf}")
                # 兼容默认名称，额外复制一份 sdk-docs.pdf，便于前端 file:// 环境无需获取项目信息
                fallback_pdf = static_dir / 'sdk-docs.pdf'
                try:
                    shutil.copy2(pdf_file, fallback_pdf)
                except Exception:
                    pass
                # 写入项目信息，供前端读取文件名
                project_info = {
                    'projectName': project_name,
                    'pdfFileName': pdf_basename
                }
                with open(static_dir / 'project_info.json', 'w', encoding='utf-8') as f:
                    json.dump(project_info, f, ensure_ascii=False)
            else:
                print("⚠️  未生成 PDF，已跳过 PDF 发布")
            
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"✗ 构建失败: {e}")
            return False
        finally:
            # 恢复到原始目录（如果不是当前分支）
            if worktree_path != Path.cwd():
                os.chdir(self.project_root)
    
    def _generate_version_config(self, output_dir: Path, version_config: VersionConfig, projects_dir_web: str = '', copy_files: list = None):
        """生成版本切换配置文件
        projects_dir_web: 仓库内项目根路径（URL 片段），例如 "project" 或 "projects/examples"
        """
        config = self.load_versions_config()
        
        # 创建版本配置 JSON 文件 - 修复格式
        version_config_file = output_dir / 'version_config.json'
        with open(version_config_file, 'w', encoding='utf-8') as f:
            json.dump(config, f, ensure_ascii=False, indent=2)
        
        # 创建版本信息 HTML 文件
        version_info_file = output_dir / 'version_info.html'
        version_info_html = f"""
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>版本信息</title>
</head>
<body>
    <script>
        window.versionInfo = {{
            "name": "{version_config.name}",
            "display_name": "{version_config.display_name}",
            "branch": "{version_config.branch}",
            "url_path": "{version_config.url_path}",
            "description": "{version_config.description}"
        }};
    </script>
</body>
</html>"""
        
        with open(version_info_file, 'w', encoding='utf-8') as f:
            f.write(version_info_html)
        
        # 同时创建 _static 目录下的配置文件
        static_dir = output_dir / '_static'
        static_dir.mkdir(exist_ok=True)
        static_config_file = static_dir / 'version_config.json'
        with open(static_config_file, 'w', encoding='utf-8') as f:
            json.dump(config, f, ensure_ascii=False, indent=2)
        # 生成可直接加载的 JS，提供 window.versionInfo（含当前版本信息）
        version_info_js = static_dir / 'version_info.js'
        version_info_obj = {
            "name": version_config.name,
            "display_name": version_config.display_name,
            "branch": version_config.branch,
            "url_path": version_config.url_path,
            "description": version_config.description,
            "projectsDir": projects_dir_web or '',
            "copyFiles": copy_files or []
        }
        with open(version_info_js, 'w', encoding='utf-8') as f:
            f.write("window.versionInfo = " + json.dumps(version_info_obj, ensure_ascii=False) + ";\n")
        
        print(f"✓ 生成版本配置文件: {version_config_file}")
        print(f"✓ 生成静态配置文件: {static_config_file}")
    
    def copy_build_result(self, worktree_path: Path, version_config: VersionConfig):
        """就地构建后无需复制，保持接口以兼容调用方"""
        target_dir = self.versions_dir / version_config.url_path
        if target_dir.exists():
            print(f"✓ 构建结果已在目标目录: {target_dir}")
            return True
        else:
            print(f"✗ 目标目录不存在: {target_dir}")
            return False

    def _generate_pdf_latex(self, docs_source_in_worktree: Path, version_config: VersionConfig) -> Optional[Path]:
        """使用传统LaTeX方法生成PDF（作为回退方案）"""
        self._ensure_pdf_dependencies()
        pdf_file = None
        
        try:
            # 尝试 latexpdf 构建器
            latexpdf_dir = self.build_root / 'latexpdf' / version_config.url_path
            print(f"尝试使用 latexpdf 构建: {latexpdf_dir}")
            subprocess.run([
                sys.executable, '-m', 'sphinx.cmd.build',
                '-b', 'latexpdf',
                str(docs_source_in_worktree),
                str(latexpdf_dir)
            ], check=True)

            # 预期输出：conf.py 设定主文档名 sdk-docs.tex -> sdk-docs.pdf
            candidate = latexpdf_dir / 'sdk-docs.pdf'
            if candidate.exists():
                pdf_file = candidate
            else:
                # 回退查找任意 pdf
                pdf_candidates = list(latexpdf_dir.glob('*.pdf'))
                if pdf_candidates:
                    pdf_file = pdf_candidates[0]
        except subprocess.CalledProcessError:
            # 回退到 latex + 编译链
            latex_dir = self.build_root / 'latex' / version_config.url_path
            print(f"latexpdf 失败，回退到 LaTeX 构建: {latex_dir}")
            subprocess.run([
                sys.executable, '-m', 'sphinx.cmd.build',
                '-b', 'latex',
                str(docs_source_in_worktree),
                str(latex_dir)
            ], check=True)

            try:
                tex_files = list(latex_dir.glob('*.tex'))
                main_tex = None
                # 优先使用 conf.py 指定的 sdk-docs.tex
                candidate_tex = latex_dir / 'sdk-docs.tex'
                if candidate_tex.exists():
                    main_tex = candidate_tex
                elif tex_files:
                    main_tex = tex_files[0]

                if main_tex:
                    # latexmk -> tectonic -> pdflatex
                    compiled = False
                    try:
                        subprocess.run(['latexmk', '-pdf', '-silent', '-interaction=nonstopmode', str(main_tex.name)], cwd=str(latex_dir), check=True)
                        compiled = True
                    except Exception:
                        try:
                            subprocess.run(['tectonic', str(main_tex.name)], cwd=str(latex_dir), check=True)
                            compiled = True
                        except Exception:
                            try:
                                subprocess.run(['pdflatex', '-interaction=nonstopmode', str(main_tex.name)], cwd=str(latex_dir), check=True)
                                subprocess.run(['pdflatex', '-interaction=nonstopmode', str(main_tex.name)], cwd=str(latex_dir), check=True)
                                compiled = True
                            except Exception:
                                pass

                    if compiled:
                        # 优先 sdk-docs.pdf
                        candidate_pdf = latex_dir / 'sdk-docs.pdf'
                        if candidate_pdf.exists():
                            pdf_file = candidate_pdf
                        else:
                            pdf_candidates = list(latex_dir.glob('*.pdf'))
                            if pdf_candidates:
                                pdf_file = pdf_candidates[0]
            except Exception as e:
                print(f"⚠️  LaTeX 回退编译失败: {e}")
        
        return pdf_file

    def _ensure_pdf_dependencies(self):
        """尽力确保本机具备 PDF 构建依赖。优先 tectonic，其次 latexmk/texlive，再次 pdflatex。
        以非交互方式尝试安装，失败仅警告不报错。
        """
        def _exists(cmd: str) -> bool:
            return which(cmd) is not None

        have_tool = _exists('tectonic') or _exists('latexmk') or _exists('pdflatex')
        have_xelatex = _exists('xelatex')
        if have_tool and have_xelatex:
            return

        system = platform.system().lower()
        print("尝试安装 PDF 构建依赖...")
        try:
            if system == 'windows':
                if _exists('choco'):
                    try:
                        subprocess.run(['choco', 'install', 'tectonic', '-y'], check=False)
                    except Exception:
                        pass
                    try:
                        subprocess.run(['choco', 'install', 'miktex', '-y'], check=False)
                    except Exception:
                        pass
            elif system == 'linux':
                if _exists('apt-get'):
                    subprocess.run(['sudo', 'apt-get', 'update'], check=False)
                    subprocess.run(['sudo', 'apt-get', 'install', '-y', 'tectonic'], check=False)
                    subprocess.run(['sudo', 'apt-get', 'install', '-y', 'latexmk', 'texlive-latex-recommended', 'texlive-latex-extra', 'texlive-fonts-recommended', 'texlive-xetex', 'fonts-noto-cjk'], check=False)
            elif system == 'darwin':
                if _exists('brew'):
                    subprocess.run(['brew', 'install', 'tectonic'], check=False)
                    subprocess.run(['brew', 'install', 'basictex'], check=False)
                    subprocess.run(['sudo', 'tlmgr', 'update', '--self'], check=False)
                    subprocess.run(['sudo', 'tlmgr', 'install', 'latexmk', 'xetex'], check=False)
        except Exception as e:
            print(f"PDF 依赖安装尝试失败: {e}")
    
    def cleanup_worktree(self, worktree_path: Path):
        """清理 worktree：仅对 source_build/worktrees 下的有效 worktree 执行删除"""
        if not worktree_path.exists():
            return

        # 仅在我们的临时 worktrees 根目录下才允许删除
        try:
            worktree_root = self.worktrees_dir.resolve()
            candidate = worktree_path.resolve()
            is_under_root = str(candidate).startswith(str(worktree_root))
        except Exception:
            is_under_root = False

        if not is_under_root:
            # 避免误删非临时目录（例如当前仓库根或任意外部路径）
            return

        # 在删除之前确认它是一个已登记的 git worktree
        is_git_worktree = False
        try:
            listed = subprocess.run(['git', 'worktree', 'list'], capture_output=True, text=True, check=True).stdout
            is_git_worktree = str(candidate) in listed
        except Exception:
            pass

        if is_git_worktree:
            # 尝试优先用 git worktree remove --force
            for args in (["git", "worktree", "remove", "--force", str(candidate)],
                         ["git", "worktree", "remove", str(candidate)]):
                try:
                    subprocess.run(args, check=True, capture_output=True)
                    print(f"✓ 清理 worktree: {worktree_path}")
                    return
                except subprocess.CalledProcessError:
                    continue

        # 兜底：非登记 worktree 或命令失败，做文件系统级别删除
        shutil.rmtree(candidate, ignore_errors=True)
    
    def build_all_versions(self, clean=False):
        """构建所有版本"""
        print("=" * 60)
        print("开始构建所有版本")
        print("=" * 60)
        
        if clean:
            print("清理构建目录...")
            if self.build_root.exists():
                shutil.rmtree(self.build_root)
        
        # 确保构建目录存在
        self.build_root.mkdir(parents=True, exist_ok=True)
        self.versions_dir.mkdir(parents=True, exist_ok=True)
        
        # 加载版本配置
        versions = self.get_version_configs()
        print(f"✓ 加载版本配置: {[v.name for v in versions]}")
        
        success_count = 0
        total_count = len(versions)
        
        for version_config in versions:
            print("\n" + "=" * 40)
            print(f"构建版本: {version_config.display_name} ({version_config.branch})")
            print("=" * 40)
            
            # 创建或获取 worktree
            worktree_path = self.create_worktree(version_config)
            if not worktree_path:
                print(f"✗ 无法为版本 {version_config.display_name} 创建 worktree")
                continue
            
            try:
                # 构建文档
                if self.build_docs_in_worktree(worktree_path, version_config):
                    # 复制构建结果
                    if self.copy_build_result(worktree_path, version_config):
                        success_count += 1
                        print(f"✓ 版本 {version_config.display_name} 构建成功")
                    else:
                        print(f"✗ 版本 {version_config.display_name} 复制失败")
                else:
                    print(f"✗ 版本 {version_config.display_name} 构建失败")
            finally:
                # 清理 worktree
                self.cleanup_worktree(worktree_path)
        
        # 创建统一入口页面，指向新的根目录结构
        self.create_unified_index()
        # 在 html 根目录下创建 index.html 指向默认版本
        self.create_versions_root_index()
        
        print("\n" + "=" * 60)
        print(f"构建完成: {success_count}/{total_count} 个版本成功")
        print("=" * 60)
        
        return success_count == total_count
    
    def create_unified_index(self):
        """创建统一的文档入口页面"""
        config = self.load_versions_config()
        versions = config.get('versions', [])
        default_version = config.get('default_version', '')
        latest_version = config.get('latest_version', '')
        
        # 找到默认版本的 URL 路径
        default_url = 'latest'
        for version in versions:
            if version['name'] == default_version:
                default_url = version['url_path']
                break
        
        # 创建根目录的 index.html
        index_html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SDK 文档</title>
    <meta http-equiv="refresh" content="0; url=./versions/{default_url}/index.html">
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
        <h1>SDK 文档</h1>
        <p>正在跳转到文档首页...</p>
        <p><a href="./versions/{default_url}/index.html">如果页面没有自动跳转，请点击这里</a></p>
    </div>
</body>
</html>"""
        
        index_file = self.build_root / 'html' / 'index.html'
        index_file.parent.mkdir(parents=True, exist_ok=True)
        
        with open(index_file, 'w', encoding='utf-8') as f:
            f.write(index_html)
        
        print(f"✓ 创建统一入口页面: {index_file}")

    def create_versions_root_index(self):
        """在versions目录下创建根页面"""
        config = self.load_versions_config()
        versions = config.get('versions', [])
        default_version = config.get('default_version', '')
        latest_version = config.get('latest_version', '')
        
        # 找到默认版本的 URL 路径
        default_url = 'latest'
        for version in versions:
            if version['name'] == default_version:
                default_url = version['url_path']
                break
        
        # 创建versions目录的index.html
        versions_index_html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SDK 文档 - 版本列表</title>
    <meta http-equiv="refresh" content="0; url=./{default_url}/index.html">
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
        <h1>SDK 文档 - 版本列表</h1>
        <p>正在跳转到文档首页...</p>
        <p><a href="./{default_url}/index.html">如果页面没有自动跳转，请点击这里</a></p>
    </div>
</body>
</html>"""
        
        versions_index_file = self.versions_dir / 'index.html'
        versions_index_file.parent.mkdir(parents=True, exist_ok=True)
        
        with open(versions_index_file, 'w', encoding='utf-8') as f:
            f.write(versions_index_html)
        
        print(f"✓ 创建versions目录根页面: {versions_index_file}")

def main():
    """主函数"""
    parser = argparse.ArgumentParser(description="中央构建管理器")
    parser.add_argument('--clean', action='store_true', help='清理构建目录')
    parser.add_argument('--list-versions', action='store_true', help='列出所有版本')
    parser.add_argument('--check-config', action='store_true', help='检查版本配置')
    
    args = parser.parse_args()
    
    try:
        manager = BuildManager()
        
        if args.list_versions:
            versions = manager.get_version_configs()
            print("版本列表:")
            for version in versions:
                print(f"  - {version.display_name} ({version.name}) -> {version.branch}")
            return
        
        if args.check_config:
            config = manager.load_versions_config()
            print("版本配置检查:")
            print(f"  默认版本: {config.get('default_version', 'N/A')}")
            print(f"  最新版本: {config.get('latest_version', 'N/A')}")
            print(f"  版本数量: {len(config.get('versions', []))}")
            return
        
        # 构建所有版本
        success = manager.build_all_versions(clean=args.clean)
        
        if success:
            print("\n🎉 所有版本构建成功!")
            print(f"📁 文档位置: {manager.versions_dir}")
        else:
            print("\n❌ 部分版本构建失败!")
            sys.exit(1)
            
    except Exception as e:
        print(f"✗ 构建管理器错误: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main() 