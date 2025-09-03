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
from pathlib import Path
from typing import List, Dict, Optional

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
        self.build_root = self.docs_source / '_build'
        self.worktrees_dir = self.build_root / 'worktrees'
        self.versions_dir = self.build_root / 'versions'
        
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
            output_dir = docs_source_in_worktree / '_build' / 'html'
            print(f"构建 HTML 文档: {output_dir}")
            subprocess.run([
                sys.executable, '-m', 'sphinx.cmd.build',
                '-b', 'html',
                str(docs_source_in_worktree),
                str(output_dir)
            ], check=True)
            
            # 生成版本配置
            self._generate_version_config(output_dir, version_config)

            # 构建 LaTeX 及 PDF（可选）
            latex_dir = docs_source_in_worktree / '_build' / 'latex'
            print(f"构建 LaTeX 文档: {latex_dir}")
            subprocess.run([
                sys.executable, '-m', 'sphinx.cmd.build',
                '-b', 'latex',
                str(docs_source_in_worktree),
                str(latex_dir)
            ], check=True)

            # 尝试编译 PDF：优先 latexmk，其次 tectonic，然后 pdflatex（可能需要多次）
            pdf_file = None
            try:
                tex_files = list(latex_dir.glob('*.tex'))
                if tex_files:
                    main_tex = tex_files[0]
                    # latexmk
                    try:
                        subprocess.run(['latexmk', '-pdf', '-silent', '-interaction=nonstopmode', str(main_tex.name)], cwd=str(latex_dir), check=True)
                    except Exception:
                        # tectonic
                        try:
                            subprocess.run(['tectonic', str(main_tex.name)], cwd=str(latex_dir), check=True)
                        except Exception:
                            # pdflatex x2 作为兜底
                            try:
                                subprocess.run(['pdflatex', '-interaction=nonstopmode', str(main_tex.name)], cwd=str(latex_dir), check=True)
                                subprocess.run(['pdflatex', '-interaction=nonstopmode', str(main_tex.name)], cwd=str(latex_dir), check=True)
                            except Exception:
                                pass

                    # 查找生成的 pdf
                    pdf_candidates = list(latex_dir.glob('*.pdf'))
                    if pdf_candidates:
                        pdf_file = pdf_candidates[0]

                # 将 PDF 复制到 HTML 的 _static 目录，供在线下载
                if pdf_file and pdf_file.exists():
                    static_dir = output_dir / '_static'
                    static_dir.mkdir(exist_ok=True)
                    target_pdf = static_dir / 'sdk-docs.pdf'
                    shutil.copy2(pdf_file, target_pdf)
                    print(f"✓ 生成并复制 PDF: {pdf_file.name} -> {target_pdf}")
                else:
                    print("⚠️  未生成 PDF（可能缺少 LaTeX 工具链），已跳过 PDF 发布")
            except Exception as e:
                print(f"⚠️  生成 PDF 过程中出现问题: {e}")
            
            return True
            
        except subprocess.CalledProcessError as e:
            print(f"✗ 构建失败: {e}")
            return False
        finally:
            # 恢复到原始目录（如果不是当前分支）
            if worktree_path != Path.cwd():
                os.chdir(self.project_root)
    
    def _generate_version_config(self, output_dir: Path, version_config: VersionConfig):
        """生成版本切换配置文件"""
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
        
        print(f"✓ 生成版本配置文件: {version_config_file}")
        print(f"✓ 生成静态配置文件: {static_config_file}")
    
    def copy_build_result(self, worktree_path: Path, version_config: VersionConfig):
        """复制构建结果到版本目录"""
        # 修复路径处理逻辑
        if worktree_path == Path.cwd():
            # 如果是当前分支，使用当前目录的构建结果
            source_dir = self.docs_source / '_build' / 'html'
        else:
            # 如果是其他分支的worktree
            source_dir = worktree_path / 'docs' / 'source' / '_build' / 'html'
        
        target_dir = self.versions_dir / version_config.url_path
        
        if source_dir.exists():
            # 清理目标目录
            if target_dir.exists():
                shutil.rmtree(target_dir)
            
            # 复制构建结果
            shutil.copytree(source_dir, target_dir)
            print(f"✓ 复制构建结果: {source_dir} -> {target_dir}")
        else:
            print(f"✗ 构建结果不存在: {source_dir}")
            return False
        
        return True
    
    def cleanup_worktree(self, worktree_path: Path):
        """清理 worktree"""
        if worktree_path.exists():
            try:
                subprocess.run(['git', 'worktree', 'remove', str(worktree_path)], 
                             check=True, capture_output=True)
                print(f"✓ 清理 worktree: {worktree_path}")
            except subprocess.CalledProcessError as e:
                print(f"⚠️  警告: 无法自动清理 worktree: {e}")
                print(f"   请手动删除: {worktree_path}")
    
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
        
        # 创建统一入口页面
        self.create_unified_index()
        
        # 在versions目录下也创建根页面
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