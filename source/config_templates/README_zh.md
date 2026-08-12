# `config.yaml` 配置模板

本目录提供两种完整配置模板。选择与仓库内容模型一致的模板，复制为 `source/config.yaml` 后修改项目元数据、源目录和分类规则。

| 模板 | 适用仓库 | 收录范围 | 导航方式 |
| --- | --- | --- | --- |
| `recursive_tree.yaml` | 教程、产品手册、知识库 | 递归同步 `projects_dir` 中受支持的文档和资源 | 按原始目录树组织 |
| `project_catalog.yaml` | SDK、BSP、示例工程集合 | 只同步匹配项目根的入口 README 和 `asset_globs` | 按 `categories` 分类组织 |

## 使用方式

```powershell
# Windows PowerShell，在仓库根目录执行
Copy-Item source/config_templates/recursive_tree.yaml source/config.yaml
cd source
python build_local.py --clean
```

```bash
# Linux/macOS，在仓库根目录执行
cp source/config_templates/recursive_tree.yaml source/config.yaml
cd source
python build_local.py --clean
```

替换已有 `source/config.yaml` 前应保留其中的项目名称、版权、giscus 和自定义字体配置。

## `recursive_tree`

该模式面向“每个目录一个 README”的文档树：

- 递归同步 `generation.sync_extensions` 允许的文件。
- 每个目录优先使用 `generation.directory_index` 指定的 README；缺失时生成索引。
- `generation.navigation.order` 中的键应与 `projects_dir` 下的顶层文档目录及 `categories` 键一致。
- PDF 保留目录 README 的一级标题，不收录其导航正文；README 是目录唯一文档时按正文处理。
- 语言识别优先使用 `projects_dir` 根 README；根目录没有语言 README 时才合并仓库根与各文档目录的标记。

## `project_catalog`

该模式面向包含大量源码、第三方包和示例项目的 SDK/BSP 仓库：

- `categories.*.patterns` 选择项目根目录。无 `/` 的模式只匹配一级目录。
- 嵌套项目必须显式配置完整相对路径，不能依赖递归 README 搜索。
- 每个项目只收录 `generation.discovery.entry_files` 和 `asset_globs`。
- 项目入口 README 是正文，会同时进入 HTML 和 PDF；供应商或包管理目录中的 README 不会进入构建。
- 不同项目可只提供一种语言；目录清单同时包含中英文入口时才显示切换 UI。
- `unmatched_projects: error` 和 `duplicate_categories: error` 建议始终保留。

严格模式还会拒绝以下配置或内容错误：

- 分类模式没有匹配任何目录。
- 一个项目同时匹配多个分类。
- 导航顺序引用未定义分类。
- 匹配项目不包含任一配置入口 README。
- 配置路径越过 `projects_dir`。
- 入口 README 引用了不存在或未被 `asset_globs` 同步的本地图片。

## 共享配置

两种模式都使用以下字段：

| 字段 | 说明 |
| --- | --- |
| `repository.projects_dir` | 相对于 `source/config.yaml` 的文档源目录，可使用 `../projects` 或 `../project` |
| `generation.default_page` | 各语言网站首页；源目录没有时使用仓库根同语言 README，并同步其中引用的本地图片 |
| `generation.default_language` | 首选语言，必须从实际可用语言中选择 |
| `generation.navigation.order` | 顶层目录或分类的显示顺序 |
| `generation.pdf_fonts` | 本地与 CI 必须安装的精确 PDF 字体；缺失时构建失败，不静默替换 |

`generation.mode`、`generation.output_structure` 仍受兼容，但新配置应以 `generation.discovery` 和 `generation.navigation` 为准。
