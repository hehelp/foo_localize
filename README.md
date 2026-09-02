# 动态多语引擎（foo_localize）

foobar2000 运行时多语言组件。不改官方程序，用外置 JSON 语言包替换界面英文。换一份语言包即可切换简体、繁体、日、俄、西、德、法等，不限于中文。英文显示名 **Dynamic Multilingual Engine**。

当前组件版本：`1.0.0`。English: [README_en.md](README_en.md)。

本仓库只托管**编译包说明**和开源的语言包 JSON，不公开插件源码。安装包在仓库的 **[Releases](https://github.com/hehelp/foo_localize/releases)** 页。

## 更新

### 1.0.0（2026-09-02）

- 首发：Windows 32 / 64 位与 macOS
- 主菜单、右键菜单、首选项、对话框、窗口标题可按语言包替换
- `查看 → 语言` 热切换；开发者模式把未译词条写入 `harvest.txt`
- 内置简中、繁中、日、俄、西、德、法及英文模板；首次启动写入 `foo-lang`（不覆盖已有文件）

## 截图

### Windows

![组件列表](screenshot/win/install-zh.png)

![查看菜单](screenshot/win/menu-item-zh.png)

![首选项](screenshot/win/setting-zh.png)

### macOS

![组件列表](screenshot/macOS/install-zh.png)

![查看菜单](screenshot/macOS/view-item-zh.png)

![首选项](screenshot/macOS/setting-zh.png)

## 支持的播放器

| 项目 | 要求 |
| --- | --- |
| 系统 | Windows 10 / 11；macOS 11+ |
| 播放器 | **foobar2000 2.0 及以上**（32 位与 64 位均可） |
| 不支持 | foobar2000 1.x；Windows 组件不能装到 Mac，反之亦然 |

32 位与 64 位是两份 DLL，不能混用。

| 播放器 | 组件文件 | 安装目录 |
| --- | --- | --- |
| foobar2000 2.x **32 位** | `foo_localize.dll` | `%APPDATA%\foobar2000-v2\user-components\foo_localize\` |
| foobar2000 2.x **64 位** | `foo_localize.dll` | `%APPDATA%\foobar2000-v2\user-components-x64\foo_localize\` |
| foobar2000 **Mac** | `foo_localize.component` | `~/Library/foobar2000-v2/user-components/` |

语言包在用户配置目录，不进官方程序目录：

| 系统 | 语言包目录 |
| --- | --- |
| Windows | `%APPDATA%\foobar2000-v2\foo-lang\` |
| macOS | `~/Library/foobar2000-v2/foo-lang\` |

## 安装

1. 打开 **[Releases](https://github.com/hehelp/foo_localize/releases)**，下载 **`foo_localize-1.0.0.fb2k-component`**（foobar 官方组件封装：一份 zip，内含 32 位、64 位与 macOS，安装时按架构自选）。
2. 在 foobar：**文件 → 首选项 → 组件 → 安装**，选中该文件。
3. 也可把对应架构的 DLL / `.component` 拷到上表目录后**完全退出再打开** foobar2000。

首次启动会把内置语言包写到 `foo-lang`（目录里还没有同名文件时才写）。**已有的 JSON 不会被覆盖**，升级组件不会丢掉你改过的译文。

64 位 Windows 常见路径：

```
C:\Users\<用户名>\AppData\Roaming\foobar2000-v2\user-components-x64\foo_localize\foo_localize.dll
C:\Users\<用户名>\AppData\Roaming\foobar2000-v2\foo-lang\
```

foobar 正在运行时无法覆盖 DLL，请先退出再拷。

## 使用

- **切语言**：`查看 → 语言`，或 **文件 → 首选项 → 显示 → 动态多语引擎**。
- **总开关 / 开发者模式**：`查看 → 动态多语引擎`。
- 开发者模式会把未翻译的界面英文追加到 `foo-lang/harvest.txt`。把新键合并进对应 JSON 后，再选一次语言或重启即可。

`en-US-template.json` 的译文全是空字符串，选它等于显示英文原文，适合当自制语言包的底板。

换语言后菜单和对话框会立刻重刷；个别自绘控件可能要再打开一次窗口才重画。

## 语言包规范

界面英文是键，译文在 JSON。空值 = 画面显示原文。不要用空字符串表示「隐藏」。

| 规则 | 说明 |
| --- | --- |
| 精确匹配 | 按界面原文查找；大小写可折叠，行首 `•` / 首尾空白会去掉后再查 |
| `@name` | 只出现在语言菜单，不参与替换 |
| 菜单加速键 | 原文里 `\t` 后面是快捷键，只译前面的可见文字 |
| 空译文 | 不翻译，保留英文 |
| 不译 | 歌名、文件名、播放列表名、路径、网址、`foobar2000`、`foo_*`、`ReFacets`、`UPnP`、`FFmpeg`、字体名、Title Formatting、配置脚本 |
| 首次写入 | 内置包只在 `foo-lang` 里**没有**该文件时写出 |
| 升级 | 不会覆盖已有 JSON；要恢复某份种子，先备份再删该文件后重启 |
| 补词 | 开发者模式 → `harvest.txt` → 合并进 JSON → 再选一次语言 |

本仓库 [`dict/`](dict/) 与组件内置种子一致，可对照或另存新语言。编写说明：[语言包编写指南](docs/lang-pack.md)。

随附文件：

| 文件 | 显示名 |
| --- | --- |
| `zh-CN.json` | 简体中文 |
| `zh-TW.json` | 繁體中文 |
| `ja-JP.json` | 日本語 |
| `ru-RU.json` | Русский |
| `es-ES.json` | Español |
| `de-DE.json` | Deutsch |
| `fr-FR.json` | Français |
| `en-US-template.json` | English (template) |

Windows **Columns UI 状态栏**音量格目前画的是 `-3.00 dB`，没有可替换的 `volume` 单词，这条暂时搁置。

## 首选项

**文件 → 首选项 → 显示 → 动态多语引擎**

可开关引擎、打开开发者模式、选择语言包。查看菜单分组名跟界面语言走：中文 **动态多语引擎**，英文 **Dynamic Multilingual Engine**。
