# 文档网站构建系统

## 概述

新的文档构建系统完全基于 `.github/versions.json` 配置文件，支持动态多版本构建，避免了分支切换导致的问题。

## 核心特性

### 1. 基于 Git Worktree 的隔离构建
- 为每个版本创建独立的 Git worktree
- 避免在主工作目录中切换分支
- 确保构建过程不会相互影响

### 2. 完全动态的版本管理
- 所有版本信息从 `.github/versions.json` 动态读取
- 无需修改脚本即可添加新版本
- 支持自动版本发现和构建

### 3. 简化的使用方式
- 统一的构建入口：`python build.py`
- 自动处理所有版本的构建
- 支持本地预览和验证

## 配置文件

### .github/versions.json
```json
{
  "versions": [
    {
      "name": "master",
      "display_name": "最新版本",
      "branch": "master",
      "url_path": "latest",
      "description": "最新开发版本"
    },
    {
      "name": "v1.0",
      "display_name": "v1.0",
      "branch": "v1.0",
      "url_path": "v1.0",
      "description": "v1.0 稳定版本"
    }
  ],
  "default_version": "master",
  "latest_version": "master"
}
```

## 使用方法

### 本地构建
```bash
# 构建所有版本
cd docs/source
python build.py

# 清理构建目录后构建
python build.py --clean

# 构建并启动本地服务器
python build.py --serve

# 验证版本配置
python build.py --validate

# 列出所有版本
python build.py --list-versions
```

### 添加新版本
1. 创建新分支：`git checkout -b v2.0`
2. 在 `.github/versions.json` 中添加版本配置：
```json
{
  "name": "v2.0",
  "display_name": "v2.0",
  "branch": "v2.0",
  "url_path": "v2.0",
  "description": "v2.0 版本"
}
```
3. 提交更改：`git commit -am "Add v2.0 version"`
4. 推送分支：`git push origin v2.0`

### GitHub Actions
- 自动检测 `.github/versions.json` 的变化
- 动态构建所有配置的版本
- 自动部署到 GitHub Pages

## 目录结构

```
docs/source/
├── build_manager.py          # 中央构建管理器
├── build.py                  # 简化构建入口
├── utils/
│   └── version_utils.py     # 版本工具
├── _build/
│   ├── worktrees/           # 临时工作树
│   │   ├── v1.0/
│   │   └── latest/
│   ├── versions/            # 最终版本文档
│   │   ├── v1.0/
│   │   └── latest/
│   └── html/                # 统一入口
└── README_NEW_BUILD_SYSTEM.md
```

## 优势

### 1. 避免分支切换问题
- 使用 Git worktree 为每个版本创建独立工作目录
- 不会影响当前分支的状态
- 构建过程更加稳定可靠

### 2. 完全动态配置
- 只需修改 `.github/versions.json` 即可添加新版本
- 无需修改任何脚本代码
- 支持任意数量的版本

### 3. 更好的隔离性
- 每个版本的构建完全独立
- 不会出现版本间的冲突
- 支持并行构建（如果资源允许）

### 4. 简化的维护
- 统一的构建入口
- 自动化的版本管理
- 清晰的错误处理和日志

## 迁移指南

### 从旧系统迁移
1. 备份当前的构建脚本
2. 确保 `.github/versions.json` 配置正确
3. 运行 `python build.py --validate` 验证配置
4. 运行 `python build.py --clean` 构建所有版本
5. 测试构建结果

### 清理旧文件
可以安全删除以下旧文件：
- `version_generator.py` (替换为 `build_manager.py`)
- `check_branch_versions.py` (功能集成到 `version_utils.py`)
- `build_local.py` (替换为 `build.py`)

## 故障排除

### 常见问题

1. **Git worktree 创建失败**
   - 确保分支存在：`git branch -a`
   - 检查 Git 版本：`git --version`

2. **版本配置验证失败**
   - 检查 `.github/versions.json` 格式
   - 确保所有必需字段都存在
   - 验证分支是否存在

3. **构建失败**
   - 检查 Python 依赖：`pip install -r requirements.txt`
   - 查看详细错误日志
   - 尝试 `--clean` 参数重新构建

### 调试命令
```bash
# 检查版本配置
python utils/version_utils.py --validate

# 列出所有版本
python utils/version_utils.py --list

# 获取当前分支
python utils/version_utils.py --current-branch

# 根据分支获取版本
python utils/version_utils.py --version-for-branch master

# 根据版本获取分支
python utils/version_utils.py --branch-for-version master
```

## 贡献指南

1. 修改版本配置时，请先运行验证命令
2. 添加新版本时，确保分支已创建并推送
3. 提交更改前，请测试构建是否成功
4. 更新文档时，请同步更新此说明文件 