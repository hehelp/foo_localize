# Component API (foo_localize)

中文：[api.md](api.md)。

The public header and a buildable sample live in [`../sdk/`](../sdk/README.md).

- Header: [`../sdk/foo_localize_api.h`](../sdk/foo_localize_api.h)
- Sample component: [`../sdk/sample/`](../sdk/sample/README.md)

Copy `foo_localize_api.h` into your project. **Do not** link any foo_localize library. If the engine is not installed, `localize_api::tryGet` returns false and the caller should keep the original string.

Existing method signatures are unchanged. To match rule objects by control context, use the new `translate_site` / COM `Translate(text, hwnd)`.

## Minimal usage

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

Use `translate_w` for a wide-character source. The result is still UTF-8. Pack ids match `foo-lang` file names, for example `zh-CN`.

```cpp
localize_api::ptr api;
if (!localize_api::tryGet(api)) {
    return;
}

pfc::string8 current;
api->get_current_language(current);

pfc::string_list_impl langs;
api->get_available_languages(langs);

api->set_language("zh-CN"); // persisted; next launch still uses Chinese
```

`set_enabled` / `set_language` write foobar `cfg_var`s. Leave switching to the user; do not change the language in the background.

`translate` / `translate_w` behave as before for plain string entries. If the key is a rule object, only `__default__` is used (`false` or empty counts as a miss). To distinguish a button from a list or a menu, use the site API below.

## Context-aware lookup (C++)

`localize_site` can set only `hwnd` (Windows fills class name and control ID lazily), or set `wnd_class` / `ctrl_id` / `menu_id` / coordinates directly. `site == nullptr` matches `translate`.

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

Ambiguous keys can be objects (keep the other 99% as `"File": "文件"`):

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

The old `translate("Title")` does not translate (default is false). `translate_site` returns “标题” on a header control and leaves list content in English.

## Query / add / edit entries (C++)

`lang_name` may be null or empty; the current pack is used. Writes are **string** values in `foo-lang/<id>.json`. The API does not build `exclude_classes`. If the key is already a rule object: `add_translation` returns false; `modify_translation` updates only `__default__`.

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

`has_translation` means the pack has a usable translation (a non-empty string, a non-empty `__default__`, or at least one class mapping). It does **not** mean “this HWND will translate right now”.

## Language-change notify

When the menu, Preferences, or another component changes the language, the engine walks every `localize_notify` and calls `on_language_changed`. The argument is valid only for that call; copy it if you need to keep it.

```cpp
class my_localize_notify : public localize_notify {
public:
    void on_language_changed(const char* new_language) override {
        (void)new_language;
        refresh_my_ui(); // do not call set_language here
    }
};

FB2K_SERVICE_FACTORY(my_localize_notify);
```

GUIDs:

| Interface | GUID |
| --- | --- |
| `localize_api` | `{5E8A1C3B-7042-4D16-9F28-A6B3D04E8C17}` |
| `localize_notify` | `{C4D29B70-1E58-4A93-86F0-2B7C5D9A4138}` |

## JS panels (Windows)

JScript Panel / Spider Monkey Panel custom drawing cannot use the C++ service above. Use COM: `FooLocalize.Engine`. There is no COM API on macOS.

Each `ActiveXObject` instance has its own panel state (`Skip`, `SetPanelType`, `SetContextType`). Create it once globally; do not `new` it inside `on_paint`.

`Translate` returns the source when the master switch is off, the dictionary misses, or this instance is inside `Skip()`. Do not pass paths or track titles.

Drawing APIs differ by panel component:

| Component | Draw text | Font | GDI hook |
| --- | --- | --- | --- |
| **JScript Panel 3** | `gr.WriteText(text, font, color, x, y, w, h)` | `JSON.stringify({Name:"Segoe UI", Size:16})` | No; wrap with `Translate` |
| Spider Monkey Panel / JSP 2 | `gr.GdiDrawText` / `gr.DrawString` | `gdi.Font(...)` | Only `GdiDrawText` goes through `DrawTextW` |

JScript Panel 3.4 has no `GdiDrawText` / `DrawString` / `gdi.Font`. Use the same style as the Klyrics panel.

`type` for `SetPanelType` / `SetContextType` is case-insensitive:

| `type` | Component switch |
| --- | --- |
| `menu` | Translate menus |
| `dialog` | Translate dialogs |
| `content` / `playlist` / `library` | Translate playlist and library (all three are the same) |

Call `SetPanelType` again at the start of every `on_paint` so the current HWND is bound to this instance. `Enable` / `Disable` / `SetLanguage` write global `cfg_var`s; call them only on an explicit user action.

JScript Panel 3.4 uses legacy JScript (ES3/ES5). Do not use `let` / `const`, arrow functions, default parameters, or object-literal method shorthand. Do not paste `[, lang]` from the docs into a script; that is not valid syntax.

### Create the object

```javascript
var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {}

function RGB(r, g, b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

var font = JSON.stringify({Name: "Segoe UI", Size: 16});

function _(text) {
    return engine ? engine.Translate(text) : text;
}
```

If the component is not installed or COM is not registered, `new ActiveXObject` throws and `engine` stays `null`. Guard every call with `engine`. On JSP3 draw with `gr.WriteText`, a JSON font string, and `RGB()`, same as the Klyrics panel.

---

### `Translate(text [, hwnd | className])`

Look up the dictionary and return the translation.

| | |
| --- | --- |
| **Parameters** | `text` (String, required): English source / dictionary key. |
| | `hwnd` (Number, optional): window handle, usually `window.ID`. The engine reads class name / control ID from that HWND and matches rule objects. |
| | `className` (String, optional): control class, e.g. `"SysHeader32"`, `"Button"`. Same slot as `hwnd` — pass one or the other. |
| **Returns** | `String`. Translation on hit; the original text when the master switch is off, this instance is inside `Skip()`, `text` is empty, or there is no match. |
| **Notes** | With only `text`, rule objects use `__default__` only (`false` or empty is a miss). A non-zero number is treated as HWND; a string is treated as a class name. |

```javascript
var label = engine.Translate("Play");
var byHwnd = engine.Translate("Play", window.ID);
var header = engine.Translate("Title", "SysHeader32");

gr.WriteText(engine.Translate("Play", window.ID), font, color, 0, 0, w, h);
```

---

### `GetLanguage()` / `IsEnabled`

Read-only. Also work as properties: `engine.GetLanguage`, `engine.IsEnabled`.

| | `GetLanguage()` | `IsEnabled` / `IsEnabled()` |
| --- | --- | --- |
| **Parameters** | none | none |
| **Returns** | `String`: current pack id, matching the `foo-lang` file name, e.g. `"zh-CN"`. Empty string if the engine is unavailable. | `Boolean`: whether the component master switch is on. |
| **Notes** | Does not change settings. | When off, hooks and `Translate` leave strings unchanged. |

```javascript
if (!engine || !engine.IsEnabled) {
    return;
}
var lang = engine.GetLanguage(); // "zh-CN"
```

---

### `Enable()` / `Disable()`

Toggle the **global** master switch. The change is persisted and applies immediately to every panel and hook.

| | |
| --- | --- |
| **Parameters** | none |
| **Returns** | `Boolean`. `true` if the engine is available (even if the value did not change); `false` if the engine is unavailable. |
| **Notes** | Call from a user button / menu, not from `on_paint`. |

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

Change the **global** current language pack, persist it, and reload the dictionary immediately.

| | |
| --- | --- |
| **Parameters** | `lang` (String, required): pack id, e.g. `"zh-CN"`, `"ja-JP"`. Must be an installed pack. |
| **Returns** | `Boolean`. `true` if the pack was applied; `false` if the id is missing, empty, or the engine is unavailable. |
| **Notes** | Call only when the user explicitly picks a language. |

```javascript
if (engine && engine.SetLanguage("zh-CN")) {
    window.Repaint();
}
```

---

### `SetPanelType(type)`

Declare the **default** translation scope for this panel. Maps to the component switches **Translate menus / dialogs / playlist and library**. Later calls replace the previous type.

| | |
| --- | --- |
| **Parameters** | `type` (String, required): `menu` / `dialog` / `content` / `playlist` / `library`. |
| **Returns** | `Boolean`. `true` if recognized; `false` if the string is unknown (state unchanged). |
| **Notes** | Affects GDI hooks for this `engine` instance (SMP `GdiDrawText`). JSP3 `WriteText` is not hooked; use `Translate`. Call again at the start of every `on_paint` so the current HWND is bound. |

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

Temporarily override the scope used by later draws, until cleared or replaced. `SetPanelType` remains the default.

| | `SetContextType(type)` | `ClearContextType([type])` |
| --- | --- | --- |
| **Parameters** | `type` (String, required): same values as `SetPanelType`. | `type` (String, optional). Omit to always clear. If passed: clear only when it matches the current context; otherwise no-op. |
| **Returns** | `Boolean`. `true` if recognized; `false` if `type` is unknown. | `Boolean`. `true` if cleared; `false` if the given `type` does not match the current context. |
| **Notes** | Later GDI hooks use this scope to decide whether to translate. On JSP3 keep calling `Translate`. | After clear, the panel falls back to `SetPanelType`. |

```javascript
engine.SetPanelType("playlist");

engine.SetContextType("menu");
gr.WriteText(engine.Translate("Settings"), font, color, 0, 0, w, rowH);
engine.ClearContextType("menu");

gr.WriteText(engine.Translate("Play"), font, color, 0, rowH, w, rowH);
```

---

### `Skip()` / `Continue()`

Skip / resume translation for **this instance**. After `Skip`, `Translate()` and SMP `GdiDrawText` → `DrawTextW` hooks leave strings unchanged until `Continue()`. JSP3 `WriteText` is not hooked; omit `Translate` for text that should stay original.

| | |
| --- | --- |
| **Parameters** | none |
| **Returns** | `Boolean`, always `true`. |
| **Notes** | Only this `engine` instance / panel. Pair the calls so the panel does not stay skipped. |

```javascript
engine.Skip();
gr.WriteText(playlistTitle, font, color, 0, 0, w, rowH);
engine.Continue();
```

---

### `HasTranslation(text)` / `HasTranslation(text, lang)`

Whether the pack already has a usable translation.

| | |
| --- | --- |
| **Parameters** | `text` (String, required): dictionary key. |
| | `lang` (String, optional): pack id. Omit or pass empty to use the current language. |
| **Returns** | `Boolean`. `true` if there is a non-empty string, a non-empty `__default__` on a rule object, or at least one class mapping. |
| **Notes** | Does **not** mean “this HWND will translate right now”. Missing key, empty value, or unknown pack id → `false`. |

```javascript
if (engine && !engine.HasTranslation("Settings")) {
    engine.AddTranslation("Settings", "设置");
}
if (engine.HasTranslation("Play", "ja-JP")) {
    // Japanese pack already has Play
}
```

---

### `AddTranslation(text, translated)` / `AddTranslation(text, translated, lang)`

**Add** a string entry. Does not overwrite an existing key.

| | |
| --- | --- |
| **Parameters** | `text` (String, required): dictionary key (English source). Empty key fails. |
| | `translated` (String, required): translation. |
| | `lang` (String, optional): which pack to write. Omit or empty → current pack. |
| **Returns** | `Boolean`. `true` on write. `false` if the key already exists (including rule objects), the pack is missing, or the engine is unavailable. |
| **Notes** | Writes a string value in `foo-lang/<id>.json`; does not build `exclude_classes`. Reloads the dictionary immediately if the active pack was changed. |

```javascript
var added = engine.AddTranslation("Settings", "设置");
var addedJa = engine.AddTranslation("Settings", "設定", "ja-JP");
```

---

### `ModifyTranslation(text, translated)` / `ModifyTranslation(text, translated, lang)`

Overwrite or insert a string entry.

| | |
| --- | --- |
| **Parameters** | `text` (String, required): dictionary key. Empty key fails. |
| | `translated` (String, required): new translation. |
| | `lang` (String, optional): which pack to write. Omit or empty → current pack. |
| **Returns** | `Boolean`. `true` on write. `false` if the pack is missing or the engine is unavailable. |
| **Notes** | Inserts the key if it does not exist. If the key is a rule object, only `__default__` is updated. |

```javascript
engine.ModifyTranslation("Settings", "设置");
engine.ModifyTranslation("Play", "播放", "zh-CN");
```

---

### Full `on_paint` demo (JScript Panel 3, same drawing style as Klyrics)

```javascript
function RGB(r, g, b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {}

var g_font = JSON.stringify({Name: "Segoe UI", Size: 16});
var g_font_hi = JSON.stringify({Name: "Segoe UI", Size: 22, Weight: 700});

function _(text) {
    return engine ? engine.Translate(text) : text;
}

function on_paint(gr) {
    var w = window.Width;
    var h = window.Height;
    gr.Clear(RGB(20, 20, 26));

    if (!engine || !engine.IsEnabled) {
        gr.WriteText("Play", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
        return;
    }

    engine.SetPanelType("playlist");

    gr.WriteText("playlist title", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
    gr.WriteText(_("Settings"), g_font, RGB(180, 180, 190), 10, 40, w - 20, 24);
    gr.WriteText(_("Play"), g_font_hi, RGB(255, 220, 80), 10, 70, w - 20, 28);
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
