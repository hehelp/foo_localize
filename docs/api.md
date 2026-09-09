# 组件 API（foo_localize）

English: [api.en.md](api.en.md)。

接口头文件和可编译示例都在 [`../sdk/`](../sdk/README.md)。

- 头文件：[`../sdk/foo_localize_api.h`](../sdk/foo_localize_api.h)
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

## JS 面板（Windows）

JScript Panel / Spider Monkey Panel 的 `gr.DrawString` 不走系统钩子，也拿不到上面的 C++ 服务。用 COM：`FooLocalize.Engine`。macOS 没有这条接口。

每个 `ActiveXObject` 实例自带一份面板状态（`Skip`、`SetPanelType`、`SetContextType`）。建议全局只创建一次，不要在 `on_paint` 里反复 `new`。

总开关关闭、词典未命中、或该实例正处于 `Skip()` 时，`Translate` 返回原文。不要把路径、曲目名传进去。`gr.DrawString` 必须自己包 `Translate`；`gr.GdiDrawText` 会走 `DrawTextW` 挂钩，同时受 `Skip` / `SetPanelType` / `SetContextType` 和词典规则约束。

`SetPanelType` / `SetContextType` 的 `type` 大小写不敏感：

| `type` | 对应组件开关 |
| --- | --- |
| `menu` | 翻译菜单 |
| `dialog` | 翻译对话框 |
| `content` / `playlist` / `library` | 翻译播放列表和媒体库（三者等价） |

建议在每次 `on_paint` 开头再调一次 `SetPanelType`，以便把当前 HWND 绑到这个实例。`Enable` / `Disable` / `SetLanguage` 会改全局 `cfg_var`，请只在用户明确操作时调用。

### 创建实例

```javascript
var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {}

function _(text) {
    return engine ? engine.Translate(text) : text;
}
```

未安装组件或 COM 未注册时，`new ActiveXObject` 会抛错，`engine` 保持 `null`。之后每条方法都先判断 `engine`。

---

### `Translate(text [, hwnd | className])`

查词典并返回译文。

| | |
| --- | --- |
| **参数** | `text`（String，必填）：英文原文，即词典键。 |
| | `hwnd`（Number，可选）：窗口句柄，通常传 `window.ID`。引擎会按该 HWND 取类名 / 控件 ID，再匹配规则对象。 |
| | `className`（String，可选）：控件类名，例如 `"SysHeader32"`、`"Button"`。与 `hwnd` 二选一，都作为第二参。 |
| **返回值** | `String`。命中则返回译文；总开关关闭、本实例 `Skip()` 中、空串、或未命中时返回原文。 |
| **说明** | 只传 `text` 时，规则对象只看 `__default__`（`false` 或空视为未命中）。第二参是数字且非 0 当作 HWND；是字符串则当作类名。 |

```javascript
var label = engine.Translate("Play");
var byHwnd = engine.Translate("Play", window.ID);
var header = engine.Translate("Title", "SysHeader32");

gr.DrawString(engine.Translate("Play", window.ID), font, color, 0, 0, w, h);
```

---

### `GetLanguage()` / `IsEnabled`

只读。也可当属性读：`engine.GetLanguage`、`engine.IsEnabled`。

| | `GetLanguage()` | `IsEnabled` / `IsEnabled()` |
| --- | --- | --- |
| **参数** | 无 | 无 |
| **返回值** | `String`：当前语言包 id，与 `foo-lang` 文件名一致，例如 `"zh-CN"`。引擎不可用时返回 `""`。 | `Boolean`：组件总开关是否打开。 |
| **说明** | 不改配置。 | 总开关关闭时，挂钩和 `Translate` 都不翻译。 |

```javascript
if (!engine || !engine.IsEnabled) {
    return;
}
var lang = engine.GetLanguage(); // "zh-CN"
```

---

### `Enable()` / `Disable()`

改**全局**总开关，写入用户配置，立刻对所有面板和挂钩生效。

| | |
| --- | --- |
| **参数** | 无 |
| **返回值** | `Boolean`。引擎可用则为 `true`（即使开关值没变）；引擎不可用为 `false`。 |
| **说明** | 只在用户点按钮 / 菜单时调用，不要在 `on_paint` 里开关。 |

```javascript
function on_button_enable() {
    if (engine) {
        engine.Enable();
    }
}

function on_button_disable() {
    if (engine) {
        engine.Disable();
    }
}
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
| **说明** | 只影响这个 `engine` 实例随后的 `GdiDrawText` 挂钩。建议每次 `on_paint` 开头再调一次，以便绑定当前 HWND。 |

```javascript
function on_paint(gr) {
    if (!engine) {
        return;
    }
    engine.SetPanelType("playlist");
    gr.GdiDrawText("Play", font, color, 0, 0, w, h, 0);
}
```

---

### `SetContextType(type)` / `ClearContextType([type])`

临时覆盖随后绘制所用的范围，直到清除或改成别的 type。`SetPanelType` 仍是默认值。

| | `SetContextType(type)` | `ClearContextType([type])` |
| --- | --- | --- |
| **参数** | `type`（String，必填）：取值同 `SetPanelType`。 | `type`（String，可选）。省略则无条件清除。传入时：与当前上下文相同才清除；不匹配则不动。 |
| **返回值** | `Boolean`。识别成功为 `true`；未知 `type` 为 `false`。 | `Boolean`。已清除为 `true`；传入的 `type` 与当前上下文不匹配为 `false`。 |
| **说明** | 之后的 `GdiDrawText` 按该范围判断是否翻译。 | 清除后回到 `SetPanelType` 的默认范围。 |

```javascript
engine.SetPanelType("playlist");

engine.SetContextType("menu");
gr.GdiDrawText("Settings", font, color, 0, 0, w, rowH, 0);
engine.ClearContextType("menu");

gr.GdiDrawText("Play", font, color, 0, rowH, w, rowH, 0);
```

---

### `Skip()` / `Continue()`

跳过 / 恢复**本实例**的翻译。`Skip` 之后，本面板的 `Translate()` 和 `GdiDrawText` → `DrawTextW` 挂钩都不再替换字符串，直到 `Continue()`。

| | |
| --- | --- |
| **参数** | 无 |
| **返回值** | `Boolean`，恒为 `true`。 |
| **说明** | 只作用于这个 `engine` 实例绑定的面板，不影响其它脚本或其它组件。务必成对调用，避免整面一直跳过。 |

```javascript
engine.Skip();
gr.GdiDrawText(playlistTitle, font, color, 0, 0, w, rowH, 0);
engine.Continue();
```

---

### `HasTranslation(text [, lang])`

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

### `AddTranslation(text, translated [, lang])`

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

### `ModifyTranslation(text, translated [, lang])`

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

### 完整 demo（`on_paint`）

```javascript
var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {}

function on_paint(gr) {
    if (!engine || !engine.IsEnabled) {
        gr.GdiDrawText(playlist, font, color, 0, 0, w, rowH, 0);
        return;
    }

    engine.SetPanelType("playlist");

    engine.Skip();
    gr.GdiDrawText(playlist, font, color, 0, 0, w, rowH, 0);
    engine.Continue();

    engine.SetContextType("menu");
    gr.GdiDrawText("Settings", font, color, 0, rowH, w, rowH, 0);
    engine.ClearContextType("menu");

    gr.DrawString(engine.Translate("Play", window.ID), font, color, 0, rowH * 2, w, rowH);
}

function on_user_add_term() {
    if (!engine) {
        return;
    }
    if (!engine.HasTranslation("Settings")) {
        engine.AddTranslation("Settings", "设置");
    }
    engine.ModifyTranslation("Settings", "设置", "zh-CN");
}
```
