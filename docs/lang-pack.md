# 语言包编写指南

English: [lang-pack.en.md](lang-pack.en.md)。

语言包是 UTF-8 JSON。**键**是 foobar2000 界面上出现的英文原文，**值**是译文或规则对象。可以按菜单/页面/文件分组写成多层对象；加载时压平成「原文 → 译文」表。同一原文出现多次时，只保留先遇到的。

首次启动组件时，内置种子会写入：

- Windows：`%APPDATA%\foobar2000-v2\foo-lang\`
- macOS：`~/Library/foobar2000-v2/foo-lang\`

**只写还不存在的文件，不覆盖已有 JSON。** 本仓库 [`dict/`](../dict/) 与内置种子相同，可对照或另存。

文件名（不含 `.json`）是语言包 id，例如 `zh-CN`。菜单里的显示名读 JSON 顶层的 `@name`。

## 文件结构

```json
{
  "@name": "简体中文",
  "File": "文件",
  "Preferences": "首选项",
  "volume": "音量"
}
```

- `@name`：只出现在语言菜单里，不参与替换。其它以 `@` 开头的键也会被跳过。
- 菜单加速键写在原文 `\t` 后面（如 `Play\tSpace`），只译 `\t` 前面的可见部分。
- 列头若带零宽空格等格式符，查词时会先去掉再匹配。`%year%` / `%length%` 也可作为键。
- 不要翻译：产品名、路径、网址、`foo_*` 组件名、歌名、字体名、Title Formatting、配置脚本。

也可以用对象分组（分组名不参与替换）。叶子仍是「原文: 值」：

```json
{
  "@name": "简体中文",
  "Top Menu": {
    "File": {
      "File": "文件",
      "Open...": "打开..."
    }
  },
  "foobar2000.exe": {
    "40001": {
      "Preferences": "首选项"
    }
  }
}
```

语言包管理窗口保存时会写成扁平 JSON，分组层级不会保留。运行时查词只认压平后的原文键。

自制新语言时，复制 `en-US-template.json`（值全空），改 `@name` 和文件名，再逐项填译文。

## 词条的值

每个原文键对应下面三种值之一。

### 1. 字符串（无条件翻译）

```json
"Play": "播放"
```

只要总开关打开、该范围（菜单 / 对话框 / 播放列表）也打开，**不论**控件类名、ID、菜单 ID、坐标，一律换成这个译文。这是最快路径，引擎不会去取窗口位置。

| 值 | 效果 |
| --- | --- |
| `"播放"` | 译成「播放」 |
| `""` | 视为未命中，画面保持英文原文。不要用空串表示「隐藏控件」 |

99% 的词条应写成字符串。只有同一句英文在不同位置含义不同、或某处不该译时，才用下面的规则对象。

### 2. 分组对象（不是词条）

对象里**没有**任何规则保留字段时，当作目录，继续往下找叶子。分组名本身不会被替换。

### 3. 规则对象（按原文出现的位置筛选）

对象里只要出现下面任一保留字段，整段就是一条规则，**不会**被当成分组拆开：

`__default__`、`when`、`exclude_classes`、`require_classes`、`exclude_ids`、`include_ids`、`exclude_menu_ids`、`positions`

没有这些字段的对象（例如 `"Title": { "SysHeader32": "标题" }`）会被当成分组，错误地拆成键 `SysHeader32`。规则对象请至少写一个 `__default__`。

无位置信息的调用（C++ `translate`、JS `Translate(text)`）**只看 `__default__`**，其它筛选项一律不生效。挂钩绘制、`translate_site`、`Translate(text, hwnd)` 才会带上位置。

位置来自 Windows 控件（类名、`GetDlgCtrlID`、菜单项 `wID`、绘制矩形）。macOS 挂钩目前拿不到 Class / ID，规则在 Mac 上基本只走 `__default__`。

#### 匹配顺序（有位置时）

按下面顺序，**先命中先返回**：

1. **`when`**：从上到下找第一条条件全部满足的条款，用它的 `text`。`text` 为 `false` 或空则**不译**（不再往后走）。
2. **类名映射**：其它键（非保留字段）若值是非空字符串，当作 Win32 类名 → 专用译文。类名大小写不敏感。命中则立刻翻译，后面的排除列表不再看。
3. **`exclude_classes`**：当前控件类名在列表里 → 不译。
4. **`exclude_ids`**：当前控件 ID 在列表里 → 不译。ID `0` 不会被排除。
5. **`exclude_menu_ids`**：当前菜单项 ID（非 0）在列表里 → 不译。
6. **`require_classes`**：列表非空，且类名不在其中 → 不译。
7. **`include_ids`**：列表非空，且控件 ID 不在其中 → 不译。
8. **`__default__`**：以上都通过后，用默认译文；为 `false`、缺省或空串则不译。

`positions`（规则顶层）会做 ±2 像素比对，但写了 `__default__` 时无论坐标准不准确都会落到第 8 步。按坐标区分译文请写在 `when` 里。

#### 同一词条多条译文：`when`

一条原文可以对应多条译文，每条带自己的条件。条款之间是「先写先用」；一条条款里写出的条件是 **AND**，数组里的多个值是 **OR**。

```json
"Play": {
  "__default__": "播放",
  "when": [
    { "class": "SysListView32", "text": false },
    { "id": 105, "text": "播放" },
    { "ids": [200, 201], "text": "演奏" },
    { "classes": ["Button", "Static"], "text": "播放" },
    { "menu_id": 40012, "text": "播放" },
    { "class": "Static", "id": 1001, "text": "播放曲目" },
    { "x": 12, "y": 8, "w": 80, "h": 24, "text": "播放" }
  ]
}
```

| 条款字段 | 类型 | 含义 |
| --- | --- | --- |
| `text` | 字符串或 `false` | 命中后的译文；`false` / 空 = 此处不译。 |
| `class` / `classes` | 字符串或字符串数组 | 控件类名，大小写不敏感。 |
| `id` / `ids` | 数字或数组 | 对话框控件 ID。 |
| `menu_id` / `menu_ids` | 数字或数组 | 菜单项 ID。 |
| `x` `y` `w` `h` | 数字 | 一条矩形，±2 像素内算命中。 |
| `pos` | `{x,y,w,h}` | 同上。 |
| `positions` | 对象数组 | 多条矩形，命中任一即可。 |

没有写的条件表示不限制。例如只写 `"id": 105` 时，不管类名是什么。没有位置的 `Translate(text)` 不跑 `when`，只用 `__default__`。

简单「某处不译、别处同一译文」仍可用下面的 `exclude_*` / 类名映射，不必上 `when`。需要**两套不同译文**时用 `when`。

#### 保留字段

| 字段 | 类型 | 含义 |
| --- | --- | --- |
| `__default__` | 字符串或 `false` | 筛选项都通过（或没有位置）时的译文。`false` / 空 = 默认不译。 |
| `when` | 对象数组 | 有序条件译文，见上一节。 |
| `exclude_classes` | 字符串数组 | 这些 Win32 类上**不要**译。常见：`SysListView32`（列表内容）、`Edit`（输入框）、`SysTreeView32`。 |
| `require_classes` | 字符串数组 | 只在这些类上译。类名对不上（或拿不到类名）则不译。 |
| `exclude_ids` | 数字或数字字符串数组 | 这些对话框控件 ID 上不要译。 |
| `include_ids` | 数字或数字字符串数组 | 只在这些控件 ID 上译。 |
| `exclude_menu_ids` | 数字或数字字符串数组 | 这些菜单项 ID 上不要译。 |
| `positions` | 对象数组 | `{ "x", "y", "w", "h" }`，与捕鱼注释里的 `Pos` 对应。见上方限制。 |
| *其它键* | 非空字符串 | 当作类名映射，例如 `"SysHeader32": "标题"`、`"Button": "播放"`。 |

类名、ID 可从开发者模式的 `harvest.txt` 抄：

```
# [Class: Button, ID: 105, MenuID: 40012, Pos: 12,8,80,24]
Play
```

- `Class` → `when` 的 `class` / `classes`，或 `exclude_classes` / `require_classes` / 类名映射
- `ID` → `when` 的 `id` / `ids`，或 `exclude_ids` / `include_ids`
- `MenuID` → `when` 的 `menu_id` / `menu_ids`，或 `exclude_menu_ids`
- `Pos` → `when` 的 `x,y,w,h` / `pos`（顶层 `positions` 仅作参考）

#### 示例

多数情况仍写 `"File": "文件"`。只有歧义时才用对象。

同一译文、只是某处不译：

```json
{
  "Play": {
    "__default__": "播放",
    "exclude_classes": ["SysListView32", "Edit"],
    "exclude_ids": [2054]
  },

  "Track": {
    "__default__": "音轨",
    "require_classes": ["Button", "Static"]
  },

  "Title": {
    "__default__": false,
    "SysHeader32": "标题"
  },

  "Preferences": {
    "__default__": "首选项",
    "exclude_menu_ids": [40099]
  }
}
```

- **Play**：按钮、菜单等译成「播放」；列表单元格、输入框、控件 2054 保持 `Play`。
- **Track**：只在 `Button` / `Static` 上译；列表里的 `Track` 不译。
- **Title**：默认不译；只有表头控件 `SysHeader32` 译成「标题」。`translate("Title")` 没有位置，因此也不译。
- **Preferences**：一般译；某个菜单 ID 除外。

同一原文、不同位置要**不同译文**时用 `when`：

```json
{
  "View": {
    "__default__": "查看",
    "when": [
      { "class": "SysListView32", "text": false },
      { "id": 105, "text": "视图" },
      { "classes": ["Button", "Static"], "text": "查看" },
      { "menu_id": 40020, "text": "查看" }
    ]
  }
}
```

- 列表内容里的 `View` 不译（多为曲目/列数据）。
- 控件 105 译成「视图」。
- 按钮和静态文本译成「查看」。
- 菜单 40020 译成「查看」。
- 对不上任何条款时用 `__default__`「查看」。
- JS / C++ 只传原文、不带位置时，只用「查看」。

JS 面板用 `Translate("Play", window.ID)` 或 `Translate("Title", "SysHeader32")` 才能走到这些筛选项。`gr.GdiDrawText` 走系统挂钩，会自动带上当前 HWND。

## 补词

1. 在 foobar：**查看 → 动态多语引擎 → 开发者模式**。
2. 点开仍显示英文的窗口或菜单。
3. 打开 **语言包管理**（菜单或首选项按钮）：左侧改当前编辑包的译文，右侧处理 `harvest.txt`，也可在线翻译原文。窗口里换包只用于编辑，不会切换 foobar2000 的翻译目标语言。
4. 也可直接打开 `foo-lang/harvest.txt`，把新行合并进对应 JSON（键 = 原文，值 = 译文或规则对象）。`# [Class: ...]` 标明控件位置，用来写 `when` / `exclude_*`。
5. 再选一次语言，或重启 foobar。

在线翻译引擎、AppID、密钥在 **首选项 → 显示 → 动态多语引擎**。公开 Google 可能限流；填写官方密钥更稳。

同一套键应在各语言文件里对齐。已有包以 `zh-CN.json` 的键集合为准。

升级组件不会改你已经改过的文件。若要恢复某份随附种子：先备份，删掉 `foo-lang` 里对应的 json，再重启。
