#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
版本生成器
根据 .github/versions.json 文件生成不同版本的文档
支持多分支文档生成
"""

import os
import sys
import shutil
import subprocess
import yaml
import json
from pathlib import Path

def load_versions():
    """从 versions.json 文件加载版本列表"""
    # 尝试多个可能的路径
    possible_paths = [
        Path("../.github/versions.json")  # 从source目录运行
    ]
    
    versions_file = None
    for path in possible_paths:
        if path.exists():
            versions_file = path
            break
    
    if not versions_file:
        print(f"错误: 版本文件不存在，尝试的路径:")
        for path in possible_paths:
            print(f"  - {path.absolute()}")
        return []
        
    try:
        with open(versions_file, 'r', encoding='utf-8') as f:
            config = json.load(f)
        
        versions = config.get('versions', [])
        print(f"加载版本配置: {[v['name'] for v in versions]}")
        return versions
    except Exception as e:
        print(f"错误: 无法解析版本配置文件: {e}")
        return []

def get_branch_name():
    """获取当前分支名称"""
    try:
        result = subprocess.run(
            ['git', 'rev-parse', '--abbrev-ref', 'HEAD'],
            capture_output=True, text=True, check=True
        )
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        print("警告: 无法获取当前分支名称")
        return None

def checkout_branch(branch_name):
    """切换到指定分支"""
    try:
        print(f"切换到分支: {branch_name}")
        subprocess.run(['git', 'checkout', branch_name], check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"错误: 无法切换到分支 {branch_name}: {e}")
        return False

def get_branch_versions():
    """获取当前分支对应的版本列表"""
    current_branch = get_branch_name()
    if not current_branch:
        return []
    
    versions = load_versions()
    branch_versions = []
    
    for version in versions:
        # 检查版本是否对应当前分支
        if version['branch'] == current_branch:
            branch_versions.append(version)
        elif version['branch'] == 'master' and current_branch == 'main':
            # 处理主分支名称差异
            branch_versions.append(version)
    
    print(f"当前分支 {current_branch} 对应的版本: {[v['name'] for v in branch_versions]}")
    return branch_versions

def build_version_docs(version_config, branch_name=None):
    """为指定版本构建文档"""
    version_name = version_config['name']
    print(f"\n开始构建版本 {version_name} 的文档...")
    
    # 确定对应的分支名称
    if branch_name is None:
        branch_name = version_config['branch']
    
    # 创建版本输出目录
    output_dir = Path(f"_build/html/{version_config['url_path']}")
    
    # 清理输出目录
    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    
    try:
        # 检查是否在GitHub Actions环境中
        is_github_actions = os.environ.get('GITHUB_ACTIONS') == 'true'
        
        # 保存当前分支名称
        current_branch = get_branch_name()
        print(f"当前分支: {current_branch}")
        
        # 如果需要切换到不同分支
        if branch_name != current_branch:
            print(f"切换到分支: {branch_name}")
            
            # 在切换分支前，先stash本地更改
            try:
                # 检查是否有未提交的更改
                result = subprocess.run(['git', 'status', '--porcelain'], 
                                     capture_output=True, text=True, check=True)
                if result.stdout and result.stdout.strip():
                    print("检测到本地更改，先stash保存...")
                    subprocess.run(['git', 'stash', 'push', '-m', f'构建前保存 - {current_branch}'], check=True)
                    print("已保存本地更改到stash")
                else:
                    print("没有未提交的更改")
            except subprocess.CalledProcessError as e:
                print(f"警告: 检查Git状态失败: {e}")
            
            # 清理可能冲突的文件
            js_file = Path("_static/version_menu.js")
            bak_file = js_file.with_suffix('.js.bak')
            
            # 删除备份文件，避免冲突
            if bak_file.exists():
                try:
                    bak_file.unlink()
                    print(f"已删除备份文件: {bak_file}")
                except Exception as e:
                    print(f"警告: 无法删除备份文件 {bak_file}: {e}")
            
            # 恢复 version_menu.js 文件（如果存在备份）
            if js_file.exists():
                try:
                    # 重置文件到Git状态
                    subprocess.run(['git', 'checkout', '--', str(js_file)], check=True)
                    print(f"已重置 {js_file} 到Git状态")
                except subprocess.CalledProcessError as e:
                    print(f"警告: 无法重置 {js_file}: {e}")
            
            # 清理构建目录以避免冲突
            build_dir = Path("_build")
            if build_dir.exists():
                try:
                    shutil.rmtree(build_dir)
                    print(f"已清理构建目录: {build_dir}")
                except PermissionError as e:
                    print(f"警告: 无法清理构建目录 {build_dir}: {e}")
                    print("尝试使用 git clean 清理...")
                    try:
                        subprocess.run(['git', 'clean', '-fd'], check=True)
                        print("已使用 git clean 清理未跟踪文件")
                    except subprocess.CalledProcessError:
                        print("警告: git clean 也失败了，继续执行...")
            
            # 清理其他可能冲突的目录
            for dir_path in ['basic', 'component', 'driver', 'protocol', 'start']:
                dir_obj = Path(dir_path)
                if dir_obj.exists():
                    try:
                        shutil.rmtree(dir_obj)
                        print(f"已清理目录: {dir_path}")
                    except Exception as e:
                        print(f"警告: 无法清理目录 {dir_path}: {e}")
            
            # 切换到目标分支
            subprocess.run(['git', 'checkout', branch_name], check=True)
            
            # 拉取最新代码
            try:
                subprocess.run(['git', 'pull', 'origin', branch_name], check=True)
                print(f"已拉取分支 {branch_name} 的最新代码")
            except subprocess.CalledProcessError as e:
                print(f"警告: 拉取代码失败: {e}")
        else:
            print(f"已在目标分支: {branch_name}")
        
        # 运行文档生成脚本
        subprocess.run([
            sys.executable, 'doc_generator.py'
        ], cwd=".", check=True)
        
        # 嵌入版本配置（自动嵌入 .github/versions.json 到 version_menu.js）
        subprocess.run([
            sys.executable, 'utils/embed_version_config.py'
        ], cwd=".", check=True)
        
        # 构建HTML文档
        subprocess.run([
            sys.executable, '-m', 'sphinx.cmd.build',
            '-b', 'html',
            '.',
            str(output_dir)
        ], cwd=".", check=True)
        
        # 生成版本切换配置文件
        generate_version_config(output_dir, version_config)
        # 写出项目信息供 PDF 下载按钮读取
        write_project_info(output_dir)
        
        # 构建完成后恢复备份文件
        subprocess.run([
            sys.executable, 'utils/embed_version_config.py', '--restore-after'
        ], cwd=".", check=True)
        
        # 如果切换了分支，恢复到原始分支
        if branch_name != current_branch:
            print(f"恢复到原始分支: {current_branch}")
            
            # 恢复 version_menu.js 文件
            js_file = Path("_static/version_menu.js")
            bak_file = js_file.with_suffix('.js.bak')
            if bak_file.exists():
                shutil.copy2(bak_file, js_file)
                print(f"已恢复 {js_file} 为备份版本")
            
            # 清理生成的文件以避免冲突
            try:
                # 强制清理所有未跟踪和修改的文件
                subprocess.run(['git', 'clean', '-fdx'], check=True)
                print("已清理未跟踪文件")
                # 重置所有修改的文件
                subprocess.run(['git', 'reset', '--hard'], check=True)
                print("已重置所有修改的文件")
            except subprocess.CalledProcessError as e:
                print(f"警告: git 清理失败: {e}")
                print("尝试手动清理...")
                # 手动清理一些关键目录
                for dir_path in ['basic', 'component', 'driver', 'protocol', 'start', '_build']:
                    dir_obj = Path(dir_path)
                    if dir_obj.exists():
                        try:
                            shutil.rmtree(dir_obj)
                            print(f"已清理目录: {dir_path}")
                        except Exception as e:
                            print(f"警告: 无法清理目录 {dir_path}: {e}")
            
            # 切换回原始分支
            subprocess.run(['git', 'checkout', current_branch], check=True)
            
            # 恢复之前stash的更改
            try:
                # 检查是否有stash
                result = subprocess.run(['git', 'stash', 'list'], 
                                     capture_output=True, text=True, check=True)
                if result.stdout and result.stdout.strip():
                    print("恢复之前保存的本地更改...")
                    subprocess.run(['git', 'stash', 'pop'], check=True)
                    print("已恢复本地更改")
                else:
                    print("没有需要恢复的stash")
            except subprocess.CalledProcessError as e:
                print(f"警告: 恢复stash失败: {e}")
        
        print(f"[OK] 版本 {version_name} 文档构建完成: {output_dir}")
        return True
        
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] 版本 {version_name} 文档构建失败: {e}")
        
        # 如果构建失败，尝试恢复到原始状态
        if branch_name != current_branch:
            try:
                print("尝试恢复到原始分支...")
                subprocess.run(['git', 'checkout', current_branch], check=True)
                
                # 恢复stash
                try:
                    result = subprocess.run(['git', 'stash', 'list'], 
                                         capture_output=True, text=True, check=True)
                    if result.stdout.strip():
                        subprocess.run(['git', 'stash', 'pop'], check=True)
                        print("已恢复本地更改")
                except subprocess.CalledProcessError:
                    pass
            except subprocess.CalledProcessError as restore_error:
                print(f"警告: 恢复原始状态失败: {restore_error}")
        
        return False



def generate_version_config(output_dir, current_version_config):
    """生成版本切换配置文件"""
    print(f"生成版本切换配置...")
    
    # 加载所有版本信息
    all_versions = load_versions()
    
    # 创建版本配置对象
    version_config = {
        "current_version": current_version_config['name'],
        "versions": {}
    }
    
    # 为每个版本创建配置
    for version in all_versions:
        version_config["versions"][version['name']] = {
            "display_name": version['display_name'],
            "url_path": version['url_path'],
            "description": version.get('description', '')
        }
    
    # 写入配置文件
    config_file = output_dir / "_static" / "version_config.json"
    config_file.parent.mkdir(parents=True, exist_ok=True)
    
    with open(config_file, 'w', encoding='utf-8') as f:
        json.dump(version_config, f, ensure_ascii=False, indent=2)
    
    print(f"[OK] 版本配置已生成: {config_file}")

def write_project_info(output_dir):
    """写出项目信息，供下载PDF按钮在 file:// 与网页模式下均可读取。
    输出到 <output_dir>/_static/project_info.json 与 project_info.js
    名称来源 source/config.yaml 的 project.name。
    """
    try:
        cfg_path = Path('config.yaml')
        project_name = 'SDK 文档'
        if cfg_path.exists():
            with open(cfg_path, 'r', encoding='utf-8') as f:
                cfg = yaml.safe_load(f) or {}
                project_name = (cfg.get('project', {}) or {}).get('name', project_name) or project_name
        info = {
            'projectName': project_name,
            'pdfFileName': f"{project_name}.pdf",
        }
        static_dir = Path(output_dir) / '_static'
        static_dir.mkdir(parents=True, exist_ok=True)
        # JSON
        with open(static_dir / 'project_info.json', 'w', encoding='utf-8') as f:
            json.dump(info, f, ensure_ascii=False)
        # JS（兼容 file:// 无法 fetch 的场景）
        try:
            with open(static_dir / 'project_info.js', 'w', encoding='utf-8') as fjs:
                fjs.write('window.projectInfo = ' + json.dumps(info, ensure_ascii=False) + ';\n')
        except Exception:
            pass
        print(f"[OK] 项目信息已生成: {static_dir / 'project_info.json'}")
    except Exception as e:
        print(f"[WARN]  生成项目信息失败: {e}")

def create_root_redirect():
    """创建根目录重定向页面"""
    print("\n创建根目录重定向页面...")
    
    # 加载版本配置
    versions = load_versions()
    default_version = None
    
    # 查找默认版本
    for version in versions:
        if version['name'] == 'master':
            default_version = version
            break
    
    if not default_version:
        print("警告: 未找到默认版本，使用第一个版本")
        default_version = versions[0] if versions else None
    
    if not default_version:
        print("错误: 没有可用的版本配置")
        return False
    
    # 创建根目录的 index.html，重定向到默认版本
    root_index = Path("_build/html/index.html")
    root_index.parent.mkdir(parents=True, exist_ok=True)
    
    redirect_html = f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SDK 文档</title>
    <meta http-equiv="refresh" content="0; url=./{default_version['url_path']}/">
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
        <p>正在跳转到{default_version['display_name']}...</p>
        <p><a href="./{default_version['url_path']}/">如果页面没有自动跳转，请点击这里</a></p>
    </div>
</body>
</html>"""
    
    with open(root_index, 'w', encoding='utf-8') as f:
        f.write(redirect_html)
    
    print(f"[OK] 根目录重定向页面创建完成: {root_index}")
    return True

def main():
    """主函数"""
    print("开始生成多版本文档...")
    
    # 检查是否在GitHub Actions环境中
    is_github_actions = os.environ.get('GITHUB_ACTIONS') == 'true'
    
    if is_github_actions:
        print("检测到GitHub Actions环境")
        # 在GitHub Actions中，为所有版本构建文档
        versions = load_versions()
        if not versions:
            print("错误: 没有找到有效的版本配置")
            return 1
        
        # 获取当前分支名称
        current_branch = get_branch_name()
        print(f"当前触发分支: {current_branch}")
        
        # 为每个版本构建文档
        results = {}
        for version_config in versions:
            # 检查分支是否存在
            try:
                subprocess.run(['git', 'ls-remote', '--heads', 'origin', version_config['branch']], 
                             capture_output=True, check=True)
                print(f"[OK] 分支 {version_config['branch']} 存在，开始构建")
                success = build_version_docs(version_config, version_config['branch'])
            except subprocess.CalledProcessError:
                print(f"[WARN]  分支 {version_config['branch']} 不存在，跳过构建")
                success = False
            results[version_config['name']] = success
    else:
        print("本地构建环境")
        # 在本地环境中，可以选择构建所有版本或只构建当前分支对应的版本
        import sys
        build_all = len(sys.argv) > 1 and sys.argv[1] == '--all'
        
        if build_all:
            print("构建所有版本...")
            versions = load_versions()
            results = {}
            for version_config in versions:
                # 检查分支是否存在
                try:
                    subprocess.run(['git', 'ls-remote', '--heads', 'origin', version_config['branch']], 
                                 capture_output=True, check=True)
                    print(f"[OK] 分支 {version_config['branch']} 存在，开始构建")
                    success = build_version_docs(version_config, version_config['branch'])
                except subprocess.CalledProcessError:
                    print(f"[WARN]  分支 {version_config['branch']} 不存在，跳过构建")
                    success = False
                results[version_config['name']] = success
            # 为每个成功版本生成项目信息（安全兜底）
            for version_config in versions:
                out_dir = Path(f"_build/html/{version_config['url_path']}")
                if out_dir.exists():
                    write_project_info(out_dir)
            for version_config in versions:
                out_dir = Path(f"_build/html/{version_config['url_path']}")
                if out_dir.exists():
                    write_project_info(out_dir)
        else:
            print("只构建当前分支对应的版本...")
            # 只构建当前分支对应的版本
            branch_versions = get_branch_versions()
            if not branch_versions:
                print("警告: 当前分支没有对应的版本配置")
                # 尝试构建默认版本
                versions = load_versions()
                branch_versions = [v for v in versions if v['name'] == 'master']
                if not branch_versions:
                    branch_versions = versions[:1] if versions else []
            
            results = {}
            for version_config in branch_versions:
                success = build_version_docs(version_config)
                results[version_config['name']] = success
            for version_config in branch_versions:
                out_dir = Path(f"_build/html/{version_config['url_path']}")
                if out_dir.exists():
                    write_project_info(out_dir)
    
    # 输出结果
    print("\n" + "="*50)
    print("版本生成结果:")
    for version, success in results.items():
        status = "[OK] 成功" if success else "[ERROR] 失败"
        print(f"  {version}: {status}")
    
    success_count = sum(1 for success in results.values() if success)
    total_count = len(results)
    
    print(f"\n总计: {success_count}/{total_count} 个版本生成成功")
    
    # 在所有版本构建完成后创建根目录重定向页面
    if success_count > 0:
        create_root_redirect()
    
    if success_count == total_count:
        print("[OK] 所有版本生成完成！")
        return 0
    else:
        print("[WARN]  部分版本生成失败，请检查错误信息")
        return 1

if __name__ == "__main__":
    sys.exit(main())
