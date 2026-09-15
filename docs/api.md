# 组件 API（foo_localize）

English: [api.en.md](api.en.md)。

接口头文件和可编译示例都在 [`../sdk/`](../sdk/README.md)。

- 头文件：[`../sdk/foo_localize_api.h`](../sdk/foo_localize_api.h)
- JS 面板示例（JScript Panel 3 / JSplitter）：[`../sdk/foo_localize.js`](../sdk/foo_localize.js)
- 示例插件：[`../sdk/sample/`](../sdk/sample/README.md)

把 `foo_localize_api.h` 拷进你的工程即可，**不要**链接任何 foo_localize 库。运行时若没装引擎，`localize_api::tryGet` 返回 false，调用方应继续用原文。

旧方法签名不变。需要按控件位置匹配规则对象时，用新增的 `translate_site` / COM `Translate(text, hwnd)`。

## 最小用法

```cpp
#include "foo_localize_api.h"

void apply_label(pfc::string_base& out, const char* english) {
    localize_api::ptr api;
    if (localize_api::tryGet(api) && api->translate(english, out)) {
        return;
    }
    out = english;
}
```

宽字符原文用 `translate_w`，译文仍是 UTF-8。语言包 id 与 `foo-lang` 文件名一致，例如 `zh-CN`。

```cpp
localize_api::ptr api;
if (!localize_api::tryGet(api)) {
    return;
}

pfc::string8 current;
api->get_current_language(current);

pfc::string_list_impl langs;
api->get_available_languages(langs);

api->set_language("zh-CN"); // 写入用户配置，下次启动仍是中文
```

`set_enabled` / `set_language` 会写入 foobar 的 `cfg_var`。请把切换权交给用户，不要在后台偷偷改语言。

`translate` / `translate_w` 对普通字符串词条与以前相同。若该键是规则对象，只看 `__default__`（`false` 或空视为未命中）。要按按钮 / 列表 / 菜单区分，用下面的 site 接口。

## 按位置翻译（C++）

`localize_site` 可只填 `hwnd`（Windows 会懒取类名和控件 ID），也可直接填 `wnd_class` / `ctrl_id` / `menu_id` / 坐标。`site == nullptr` 与 `translate` 相同。

```cpp
void apply_label_at(HWND hwnd, pfc::string_base& out, const char* english) {
    localize_api::ptr api;
    if (!localize_api::tryGet(api)) {
        out = english;
        return;
    }
    localize_api::localize_site site;
    site.hwnd = hwnd;
    if (api->translate_site(english, &site, out)) {
        return;
    }
    out = english;
}
```

词典里可以这样写歧义词（其余 99% 仍用 `"File": "文件"`）：

```json
{
  "Play": {
    "__default__": "播放",
    "exclude_classes": ["SysListView32", "Edit"]
  },
  "Title": {
    "__default__": false,
    "SysHeader32": "标题"
  }
}
```

旧 `translate("Title")` 不会译（默认 false）。`translate_site` 在表头控件上会得到「标题」，在列表内容上保持原文。

## 查 / 加 / 改词条（C++）

`lang_name` 可空，空则用当前语言包。写入的是 `foo-lang/<id>.json` 的**字符串**值；不会从接口拼 `exclude_classes`。键已是规则对象时：`add_translation` 返回 false；`modify_translation` 只改 `__default__`。

```cpp
localize_api::ptr api;
if (!localize_api::tryGet(api)) {
    return;
}

if (!api->has_translation("Play", nullptr)) {
    api->add_translation("Play", "播放", nullptr);
}

api->modify_translation("Play", "播放", "zh-CN");
```

`has_translation` 表示包里有可用译文（非空字符串，或规则的非空 `__default__`，或至少一条 class 映射），**不**表示「这个 HWND 当下会不会译」。

## 监听语言变化

语言被菜单、首选项或其他组件改掉时，引擎会遍历所有 `localize_notify` 并调用 `on_language_changed`。参数只在这次调用内有效，需要保存请立刻拷走。

```cpp
class my_localize_notify : public localize_notify {
public:
    void on_language_changed(const char* new_language) override {
        (void)new_language;
        refresh_my_ui(); // 不要在这里再调 set_language
    }
};

FB2K_SERVICE_FACTORY(my_localize_notify);
```

GUID：

| 接口 | GUID |
| --- | --- |
| `localize_api` | `{5E8A1C3B-7042-4D16-9F28-A6B3D04E8C17}` |
| `localize_notify` | `{C4D29B70-1E58-4A93-86F0-2B7C5D9A4138}` |

## Zero Bus（Windows / macOS）

在首选项 **动态多语引擎 → Zero Bus 服务** 打开「启用Zero Bus 服务」，并安装 [`foo_zero_bus`](https://github.com/hehelp/foo_zero_bus) 后，本组件注册服务 **`plugin.localize`**。WebSocket / 其它组件可用同一套 JSON：`cmd` / `ok` / `event`。未安装 Zero Bus 时本组件照常工作。

完整 cmd 表：[zero-bus.md](zero-bus.md)。

```js
payload: JSON.stringify({ cmd: "translate", text: "Play" })
// 信封再 JSON.stringify 一次；receiver 填 plugin.localize
```

`translate` 未命中返回 `{"ok":true,"hit":false}`，**不**回原文（与上面的 C++ API 一致）。语言实际变更时广播 `{"event":"language_changed","lang":"zh-CN"}`。

本服务没有 COM 的 `Skip` / `SetPanelType` / 按 hwnd 换字闸。COM 仍仅 Windows。

## JS 面板（Windows）

JScript Panel / Spider Monkey Panel 自绘文字不走 C++ 服务，用 COM：`FooLocalize.Engine`。macOS 没有这条接口。

每个 `ActiveXObject` 实例自带一份面板状态（`Skip`、`SetPanelType`、`SetContextType`）。建议全局只创建一次，不要在 `on_paint` 里反复 `new`。

`Translate` 只受查词闸约束：`DisableTranslation()`、词典未命中、或空串时回原文。它**不**看 `Enable`/`Disable`（换字）和「启用多语引擎」。不要把路径、曲目名传进去。

画字语法按面板组件区分：

| 组件 | 画字 | 字体 | 是否走 GDI 挂钩 |
| --- | --- | --- | --- |
| **JScript Panel 3** | `gr.WriteText(text, font, color, x, y, w, h)` | `JSON.stringify({Name:"Segoe UI", Size:16})` | 否，必须自己包 `Translate` |
| Spider Monkey Panel / JSP 2 | `gr.GdiDrawText` / `gr.DrawString` | `gdi.Font(...)` | 仅 `GdiDrawText` 会走 `DrawTextW` |

JScript Panel 3.4 没有 `GdiDrawText` / `DrawString` / `gdi.Font`。写法与快乐歌词面板相同。

`SetPanelType` / `SetContextType` 的 `type` 大小写不敏感：

| `type` | 对应组件开关 |
| --- | --- |
| `menu` | 翻译菜单 |
| `dialog` | 翻译对话框 |
| `content` / `playlist` / `library` | 翻译播放列表和媒体库（三者等价） |

建议在每次 `on_paint` 开头再调一次 `SetPanelType`，以便把当前 HWND 绑到这个实例。`SetLanguage` 会改全局语言包配置，请只在用户明确选语言时调用。`Enable` / `Disable` 只改换字，不写「启用多语引擎」。

#### 换字 vs 查词（不要混）

| | `Enable` / `Disable` | `EnableTranslation` / `DisableTranslation` |
| --- | --- | --- |
| **管什么** | **换字**：挂钩要不要把界面原文换成译文 | **查词**：`Translate()` 要不要查词典 |
| **作用范围** | 全局，或 `Enable(hwnd)` / `Disable(hwnd)` 只覆盖一个窗口 | 这个 `engine` 实例 |
| **关了之后** | 菜单/对话框等不再被挂钩替换；`Translate()` **仍能**出译文 | `Translate()` 一律回原文；挂钩换字**不受影响** |
| **不管什么** | 不装/卸挂钩，不改用户「启用多语引擎」勾选 | 不改换字，不改插件总闸 |

只有用户在首选项里打开「启用多语引擎」，挂钩才会动态换字。脚本用 `IsPluginEnabled()` 读这个勾选，不能改它。

JScript Panel 3.4 用的是旧版 JScript（ES3/ES5）。不要用 `let` / `const`、箭头函数、默认参数、对象方法简写。也不要把文档里的 `[, lang]` 抄进脚本，那不是合法语法。

### 创建实例

JScript Panel 3 与 JSplitter / SMP 都用同一段。初始化时会在控制台打 `ok` / `FAIL`，便于发现宿主差异。方法一律带括号。完整可运行脚本见 [`../sdk/foo_localize.js`](../sdk/foo_localize.js)。

```javascript
function log(msg) {
    try {
        console.log(msg);
    } catch (ignored) {}
}

function check(name, ok, extra) {
    log((ok ? "ok  " : "FAIL") + " " + name + (extra ? " " + extra : ""));
}

var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {
    log("FooLocalize FAIL: " + e.message);
}

if (engine) {
    try {
        var lang = engine.GetLanguage();
        check("GetLanguage", typeof lang === "string" && lang.length > 0, lang);
        check("IsEnabled", engine.IsEnabled() === true || engine.IsEnabled() === false, String(engine.IsEnabled()));
        check("IsPluginEnabled", engine.IsPluginEnabled() === true || engine.IsPluginEnabled() === false);
        check("IsEnabledTranslation", engine.IsEnabledTranslation() === true);
        check("Translate", typeof engine.Translate("Play") === "string", engine.Translate("Play"));
        check("Translate class", typeof engine.Translate("Title", "SysHeader32") === "string");
        check("Translate hwnd", typeof engine.Translate("Play", window.ID) === "string");
        check("SetPanelType", engine.SetPanelType("playlist") === true);
        check("SetPanelType bad", engine.SetPanelType("nope") === false);
        check("SetContextType", engine.SetContextType("menu") === true);
        check("ClearContextType", engine.ClearContextType("menu") === true);
        check("ClearContextType()", engine.ClearContextType() === true);
        engine.DisableTranslation();
        check("DisableTranslation", engine.Translate("Play") === "Play");
        engine.EnableTranslation();
        check("EnableTranslation", engine.Translate("Play") !== "");
        engine.Skip();
        check("Skip still Translate", engine.Translate("Play") !== "");
        engine.Continue();
        check("HasTranslation", engine.HasTranslation("Play") === true || engine.HasTranslation("Play") === false);
        check("HasTranslation lang", engine.HasTranslation("Play", lang) === true || engine.HasTranslation("Play", lang) === false);
        var key = "FooLocalizeJsTest";
        engine.ModifyTranslation(key, "test", lang);
        check("ModifyTranslation", engine.HasTranslation(key, lang) === true);
    } catch (e) {
        log("FooLocalize FAIL: " + e.message);
        try {
            engine.Continue();
        } catch (ignored) {}
    }
}

function _(text) {
    return engine ? engine.Translate(text) : text;
}
```

未安装组件或 COM 未注册时，`new ActiveXObject` 会抛错，`engine` 保持 `null`。之后每条方法都先判断 `engine`。JSP3 画字用 `gr.WriteText`；JSplitter / SMP 用 `gr.GdiDrawText` 和 `gdi.Font`。SDK 示例会按宿主自动选。`ModifyTranslation` 自检会写入键 `FooLocalizeJsTest`。

---

### `Translate(text [, hwnd | className])`

查词典并返回译文。

| | |
| --- | --- |
| **参数** | `text`（String，必填）：英文原文，即词典键。 |
| | `hwnd`（Number，可选）：窗口句柄，通常传 `window.ID`。引擎会按该 HWND 取类名 / 控件 ID，再匹配规则对象。 |
| | `className`（String，可选）：控件类名，例如 `"SysHeader32"`、`"Button"`。与 `hwnd` 二选一，都作为第二参。 |
| **返回值** | `String`。命中则返回译文；查词闸关闭、空串、或未命中时返回原文。 |
| **说明** | 不依赖「启用多语引擎」和 `IsEnabled()` 换字闸。只传 `text` 时，规则对象只看 `__default__`（`false` 或空视为未命中）。第二参是数字且非 0 当作 HWND；是字符串则当作类名。 |

```javascript
var label = engine.Translate("Play");
var byHwnd = engine.Translate("Play", window.ID);
var header = engine.Translate("Title", "SysHeader32");

gr.WriteText(engine.Translate("Play", window.ID), font, color, 0, 0, w, h);
```

---

### `GetLanguage()` / `IsEnabled([hwnd])` / `IsPluginEnabled()`

只读。JSplitter / SMP 必须带括号当方法调。

| | `GetLanguage()` | `IsEnabled([hwnd])` | `IsPluginEnabled()` |
| --- | --- | --- | --- |
| **参数** | 无 | `hwnd`（Number，可选） | 无 |
| **返回值** | 当前语言包 id，引擎不可用时 `""` | 无参：全局**换字**位。有 hwnd：该窗口现在会不会被挂钩换字（插件关则为 `false`） | 首选项里「启用多语引擎」是否打开 |
| **说明** | 不改配置。 | **不再**表示插件总闸。插件关着时挂钩不换字，但 `Translate` 仍可查词。 | 只读用户勾选。脚本不能改这个位。 |

```javascript
var lang = engine.GetLanguage(); // "zh-CN"
var hooksOn = engine.IsPluginEnabled();
var replaceAll = engine.IsEnabled();
var thisPanel = engine.IsEnabled(window.ID);
```

---

### `Enable([hwnd])` / `Disable([hwnd])`

**只控制换字，不控制查词。** 挂钩要不要把界面原文换成译文。不改首选项里的「启用多语引擎」，也不装/卸挂钩。要关 `Translate()` 请用下面的 `DisableTranslation()`。

| | |
| --- | --- |
| **参数** | `hwnd`（Number，可选）。省略则改全局换字位。传入 `window.ID` 则只覆盖该窗口（及直接子窗口）。 |
| **返回值** | `Boolean`。引擎可用且参数有效为 `true`。 |
| **说明** | 插件关着时改换字位只记账，等用户打开插件后才动态换字。`Disable()` 后整机不再换字，但 `Translate()` 仍可用。`Enable(window.ID)` 可在全局换字关闭时只译本面板。不要在 `on_paint` 里反复开关。 |

```javascript
engine.Disable();
engine.Enable(window.ID);
var byHwnd = engine.Translate("Play", window.ID);
```

---

### `EnableTranslation()` / `DisableTranslation()` / `IsEnabledTranslation()`

**只控制查词，不控制换字。** 本实例的 `Translate()` 要不要查词典。与 `Enable`/`Disable`、插件总闸都独立。默认开。要停挂钩换字请用上面的 `Disable()` / `Disable(hwnd)`。

| | |
| --- | --- |
| **参数** | 无 |
| **返回值** | `Boolean`。引擎可用为 `true`（`IsEnabledTranslation` 另表示闸是否开）。 |
| **说明** | 关掉后 `Translate` 原样返回。不影响挂钩换字。 |

```javascript
engine.DisableTranslation();
engine.Translate("Play"); // "Play"
engine.EnableTranslation();
```

---

### `SetLanguage(lang)`

改**全局**当前语言包，写入用户配置并立刻重载词典。

| | |
| --- | --- |
| **参数** | `lang`（String，必填）：语言包 id，例如 `"zh-CN"`、`"ja-JP"`。必须是已安装的包。 |
| **返回值** | `Boolean`。成功应用为 `true`；id 不存在、空串或引擎不可用为 `false`。 |
| **说明** | 只在用户明确选语言时调用。 |

```javascript
if (engine && engine.SetLanguage("zh-CN")) {
    window.Repaint();
}
```

---

### `SetPanelType(type)`

声明**本面板**默认翻译范围，对应组件里的「翻译菜单 / 对话框 / 播放列表和媒体库」。多次调用覆盖上一次。

| | |
| --- | --- |
| **参数** | `type`（String，必填）：`menu` / `dialog` / `content` / `playlist` / `library`。 |
| **返回值** | `Boolean`。识别成功为 `true`；未知字符串为 `false`，状态不变。 |
| **说明** | 只影响这个 `engine` 实例随后的 GDI 挂钩（SMP 的 `GdiDrawText`）。JSP3 的 `WriteText` 不挂钩，请用 `Translate`。建议每次 `on_paint` 开头再调一次，以便绑定当前 HWND。 |

```javascript
function on_paint(gr) {
    if (!engine) {
        return;
    }
    engine.SetPanelType("playlist");
    gr.WriteText(engine.Translate("Play"), font, color, 0, 0, w, h);
}
```

---

### `SetContextType(type)` / `ClearContextType([type])`

临时覆盖随后绘制所用的范围，直到清除或改成别的 type。`SetPanelType` 仍是默认值。

| | `SetContextType(type)` | `ClearContextType([type])` |
| --- | --- | --- |
| **参数** | `type`（String，必填）：取值同 `SetPanelType`。 | `type`（String，可选）。省略则无条件清除。传入时：与当前上下文相同才清除；不匹配则不动。 |
| **返回值** | `Boolean`。识别成功为 `true`；未知 `type` 为 `false`。 | `Boolean`。已清除为 `true`；传入的 `type` 与当前上下文不匹配为 `false`。 |
| **说明** | 之后的 GDI 挂钩按该范围判断是否翻译。JSP3 请继续用 `Translate`。 | 清除后回到 `SetPanelType` 的默认范围。 |

```javascript
engine.SetPanelType("playlist");

engine.SetContextType("menu");
gr.WriteText(engine.Translate("Settings"), font, color, 0, 0, w, rowH);
engine.ClearContextType("menu");

gr.WriteText(engine.Translate("Play"), font, color, 0, rowH, w, rowH);
```

---

### `Skip()` / `Continue()`

跳过 / 恢复**本实例**的挂钩换字（如 SMP `GdiDrawText` → `DrawTextW`）。不再影响 `Translate()`。JSP3 的 `WriteText` 不挂钩，不译的字符串不要包 `Translate`，或用 `DisableTranslation()`。

| | |
| --- | --- |
| **参数** | 无 |
| **返回值** | `Boolean`，恒为 `true`。 |
| **说明** | 只作用于这个 `engine` 实例绑定的面板，不影响其它脚本或其它组件。务必成对调用，避免整面一直跳过。 |

```javascript
engine.Skip();
gr.WriteText(playlistTitle, font, color, 0, 0, w, rowH);
engine.Continue();
```

---

### `HasTranslation(text)` / `HasTranslation(text, lang)`

查语言包里是否已有可用译文。

| | |
| --- | --- |
| **参数** | `text`（String，必填）：词典键。 |
| | `lang`（String，可选）：语言包 id。省略或空则用当前语言。 |
| **返回值** | `Boolean`。有非空字符串、或规则对象有非空 `__default__`、或至少有一条 class 映射时为 `true`。 |
| **说明** | **不**表示「这个 HWND 当下会不会译」。键不存在、空译文、或语言包 id 无效时为 `false`。 |

```javascript
if (engine && !engine.HasTranslation("Settings")) {
    engine.AddTranslation("Settings", "设置");
}
if (engine.HasTranslation("Play", "ja-JP")) {
    // 日语包里已有 Play
}
```

---

### `AddTranslation(text, translated)` / `AddTranslation(text, translated, lang)`

向语言包**新增**一条字符串词条，不覆盖已有键。

| | |
| --- | --- |
| **参数** | `text`（String，必填）：词典键（英文原文）。空串失败。 |
| | `translated`（String，必填）：译文。 |
| | `lang`（String，可选）：写入哪个包。省略或空则写当前语言包。 |
| **返回值** | `Boolean`。写入成功为 `true`。键已存在（含规则对象）、语言包不存在、或引擎不可用为 `false`。 |
| **说明** | 写入 `foo-lang/<id>.json` 的字符串值，不会拼 `exclude_classes`。当前语言包被改到时会立刻重载词典。 |

```javascript
var added = engine.AddTranslation("Settings", "设置");
var addedJa = engine.AddTranslation("Settings", "設定", "ja-JP");
```

---

### `ModifyTranslation(text, translated)` / `ModifyTranslation(text, translated, lang)`

覆盖或新增一条字符串词条。

| | |
| --- | --- |
| **参数** | `text`（String，必填）：词典键。空串失败。 |
| | `translated`（String，必填）：新译文。 |
| | `lang`（String，可选）：写入哪个包。省略或空则写当前语言包。 |
| **返回值** | `Boolean`。写入成功为 `true`。语言包不存在或引擎不可用为 `false`。 |
| **说明** | 键不存在则新增。键已是规则对象时只改 `__default__`，其它字段不动。 |

```javascript
engine.ModifyTranslation("Settings", "设置");
engine.ModifyTranslation("Play", "播放", "zh-CN");
```

---

### 完整 demo（JScript Panel 3 / JSplitter）

与 [`../sdk/foo_localize.js`](../sdk/foo_localize.js) 相同。启动时先自检接口。有 `WriteText` 走 JSP3 画法，否则走 JSplitter / SMP 的 `GdiDrawText`。

```javascript
function RGB(r, g, b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

function log(msg) {
    try {
        console.log(msg);
    } catch (ignored) {}
}

function check(name, ok, extra) {
    log((ok ? "ok  " : "FAIL") + " " + name + (extra ? " " + extra : ""));
}

var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {
    log("FooLocalize FAIL: " + e.message);
}

if (engine) {
    try {
        var lang = engine.GetLanguage();
        check("GetLanguage", typeof lang === "string" && lang.length > 0, lang);
        check("IsEnabled", engine.IsEnabled() === true || engine.IsEnabled() === false, String(engine.IsEnabled()));
        check("IsPluginEnabled", engine.IsPluginEnabled() === true || engine.IsPluginEnabled() === false);
        check("IsEnabledTranslation", engine.IsEnabledTranslation() === true);
        check("Translate", typeof engine.Translate("Play") === "string", engine.Translate("Play"));
        check("Translate class", typeof engine.Translate("Title", "SysHeader32") === "string");
        check("Translate hwnd", typeof engine.Translate("Play", window.ID) === "string");
        check("SetPanelType", engine.SetPanelType("playlist") === true);
        check("SetPanelType bad", engine.SetPanelType("nope") === false);
        check("SetContextType", engine.SetContextType("menu") === true);
        check("ClearContextType", engine.ClearContextType("menu") === true);
        check("ClearContextType()", engine.ClearContextType() === true);
        engine.DisableTranslation();
        check("DisableTranslation", engine.Translate("Play") === "Play");
        engine.EnableTranslation();
        check("EnableTranslation", engine.Translate("Play") !== "");
        engine.Skip();
        check("Skip still Translate", engine.Translate("Play") !== "");
        engine.Continue();
        check("HasTranslation", engine.HasTranslation("Play") === true || engine.HasTranslation("Play") === false);
        check("HasTranslation lang", engine.HasTranslation("Play", lang) === true || engine.HasTranslation("Play", lang) === false);
        var key = "FooLocalizeJsTest";
        engine.ModifyTranslation(key, "test", lang);
        check("ModifyTranslation", engine.HasTranslation(key, lang) === true);
    } catch (e) {
        log("FooLocalize FAIL: " + e.message);
        try {
            engine.Continue();
        } catch (ignored) {}
    }
}

var g_jsp3 = typeof gdi === "undefined" || !gdi.Font;
var g_font = g_jsp3
    ? JSON.stringify({Name: "Segoe UI", Size: 16})
    : gdi.Font("Segoe UI", 16, 0);
var g_font_hi = g_jsp3
    ? JSON.stringify({Name: "Segoe UI", Size: 22, Weight: 700})
    : gdi.Font("Segoe UI", 22, 1);

function _(text) {
    return engine ? engine.Translate(text) : text;
}

function draw_text(gr, text, font, color, x, y, w, h) {
    if (g_jsp3) {
        gr.WriteText(text, font, color, x, y, w, h);
        return;
    }
    gr.GdiDrawText(text, font, color, x, y, w, h, 0);
}

function on_paint(gr) {
    var w = window.Width;
    var h = window.Height;
    if (g_jsp3) {
        gr.Clear(RGB(20, 20, 26));
    } else {
        gr.FillSolidRect(0, 0, w, h, RGB(20, 20, 26));
    }

    if (!engine) {
        draw_text(gr, "Play", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
        return;
    }

    engine.SetPanelType("playlist");
    draw_text(gr, "playlist title", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
    draw_text(gr, _("Settings"), g_font, RGB(180, 180, 190), 10, 40, w - 20, 24);
    draw_text(gr, _("Play"), g_font_hi, RGB(255, 220, 80), 10, 70, w - 20, 28);
}

function on_mouse_lbtn_up(x, y) {
    if (!engine) {
        return;
    }
    if (!engine.HasTranslation("Settings")) {
        engine.AddTranslation("Settings", "Shezhi");
    }
    engine.ModifyTranslation("Settings", "Shezhi", "zh-CN");
    window.Repaint();
}
```
