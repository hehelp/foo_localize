# Language pack guide

中文：[lang-pack.md](lang-pack.md)。

A language pack is UTF-8 JSON. The **key** is the English UI string shown by foobar2000; the **value** is the translation or a rule object. You may nest objects by menu, page, or file; on load they are flattened to a source → translation map. If the same source appears more than once, the first occurrence wins.

On first launch the built-in seeds are written to:

- Windows: `%APPDATA%\foobar2000-v2\foo-lang\`
- macOS: `~/Library/foobar2000-v2/foo-lang/`

**A file is written only if it does not already exist.** Existing JSON is never overwritten. [`dict/`](../dict/) in this repo matches the built-in seeds.

The file name without `.json` is the pack id (for example `zh-CN`). The name in the Language menu comes from top-level `@name`.

## File layout

```json
{
  "@name": "English (template)",
  "File": "",
  "Preferences": "",
  "volume": ""
}
```

- `@name` appears only in the language menu and is not substituted into the UI. Other keys that start with `@` are skipped as well.
- Menu accelerators sit after `\t` in the source (e.g. `Play\tSpace`). Translate only the visible part before `\t`.
- Column headers may contain zero-width format characters; those are stripped before lookup. `%year%` / `%length%` wrappers are also accepted as keys.
- Do not translate product names, paths, URLs, `foo_*` component ids, track titles, font names, Title Formatting, or config scripts.

You may also group with objects (group names are not substituted). Leaves are still `source: value`:

```json
{
  "@name": "English (template)",
  "Top Menu": {
    "File": {
      "File": "",
      "Open...": ""
    }
  },
  "foobar2000.exe": {
    "40001": {
      "Preferences": ""
    }
  }
}
```

Saving from the pack manager writes a flat JSON file; grouping is not preserved. Runtime lookup only uses the flattened source keys.

To start a new language, copy `en-US-template.json` (all values empty), change `@name` and the file name, then fill in translations.

## Entry values

Each source key has one of the three value kinds below.

### 1. String (unconditional)

```json
"Play": "播放"
```

If the master switch is on and the scope (menus / dialogs / playlist) is on, this string is used **regardless** of window class, control ID, menu ID, or coordinates. This is the fast path: the engine does not read control context.

| Value | Effect |
| --- | --- |
| `"播放"` | Replace with that text |
| `""` | Miss: the UI keeps the English source. Do not use empty to mean “hide the control” |

Use a plain string for about 99% of entries. Use a rule object only when the same English text means different things in different places, or must not be translated in some of them.

### 2. Group object (not an entry)

If the object has **none** of the reserved rule fields, it is a folder. The engine walks into it. The folder name itself is not substituted.

### 3. Rule object (filter by where the source appears)

If the object contains any of these reserved fields, it is one rule and is **not** flattened as a group:

`__default__`, `when`, `exclude_classes`, `require_classes`, `exclude_ids`, `include_ids`, `exclude_menu_ids`, `positions`

An object without those fields (for example `"Title": { "SysHeader32": "标题" }`) is treated as a group and incorrectly becomes the key `SysHeader32`. Always include at least `__default__` on a rule.

Calls with no site (C++ `translate`, JS `Translate(text)`) use **`__default__` only**. The other filters do nothing. Hooked painting, `translate_site`, and `Translate(text, hwnd)` pass a site.

The site comes from the Windows control (class name, `GetDlgCtrlID`, menu item `wID`, paint rectangle). macOS hooks currently cannot read Class / ID, so on Mac a rule almost always falls back to `__default__`.

#### Match order (when a site is present)

First match wins:

1. **`when`**: first clause whose conditions all match. Use its `text`. `false` or empty → **do not translate** (stop; do not fall through).
2. **Class map**: any other key (not reserved) whose value is a non-empty string is a Win32 class name → specific translation. Class names are case-insensitive. A hit translates immediately; exclude lists are not applied.
3. **`exclude_classes`**: current class is in the list → do not translate.
4. **`exclude_ids`**: current control ID is in the list → do not translate. ID `0` is never excluded.
5. **`exclude_menu_ids`**: current menu item ID (non-zero) is in the list → do not translate.
6. **`require_classes`**: list is non-empty and the class is not in it → do not translate.
7. **`include_ids`**: list is non-empty and the control ID is not in it → do not translate.
8. **`__default__`**: if every check above passed, use the default; `false`, missing, or empty means do not translate.

Top-level `positions` compares rectangles with a ±2 pixel tolerance, but a non-empty `__default__` is still used when the rectangle does not match. Put coordinate-specific translations in `when`.

#### Several translations for one key: `when`

One source string may have several translations, each with its own conditions. Clauses are first-match. Conditions written on one clause are **AND**; values inside an array are **OR**.

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

| Clause field | Type | Meaning |
| --- | --- | --- |
| `text` | string or `false` | Translation on hit; `false` / empty = leave the source here. |
| `class` / `classes` | string or array | Control class, case-insensitive. |
| `id` / `ids` | number or array | Dialog control ID. |
| `menu_id` / `menu_ids` | number or array | Menu item ID. |
| `x` `y` `w` `h` | numbers | One rectangle; match within ±2 pixels. |
| `pos` | `{x,y,w,h}` | Same as inline x/y/w/h. |
| `positions` | array of objects | Any rectangle may match. |

Omitted conditions are unrestricted. `{ "id": 105 }` matches that ID on any class. `Translate(text)` has no site, so it skips `when` and uses `__default__` only.

Use `exclude_*` / a class map when the other places share one translation. Use `when` when you need **different** translations.

#### Reserved fields

| Field | Type | Meaning |
| --- | --- | --- |
| `__default__` | string or `false` | Translation when the filters pass (or when there is no site). `false` / empty = do not translate by default. |
| `when` | array of objects | Ordered conditional translations; see the previous section. |
| `exclude_classes` | array of strings | Do **not** translate on these Win32 classes. Typical: `SysListView32` (list content), `Edit` (edit box), `SysTreeView32`. |
| `require_classes` | array of strings | Translate only on these classes. Missing or other class → no translation. |
| `exclude_ids` | array of numbers or numeric strings | Do not translate on these dialog control IDs. |
| `include_ids` | array of numbers or numeric strings | Translate only on these control IDs. |
| `exclude_menu_ids` | array of numbers or numeric strings | Do not translate on these menu item IDs. |
| `positions` | array of objects | `{ "x", "y", "w", "h" }`, matching `Pos` in harvest comments. See the limitation above. |
| *any other key* | non-empty string | Class map, e.g. `"SysHeader32": "标题"`, `"Button": "播放"`. |

Copy class / ID from Developer Mode `harvest.txt`:

```
# [Class: Button, ID: 105, MenuID: 40012, Pos: 12,8,80,24]
Play
```

- `Class` → `when` `class` / `classes`, or `exclude_classes` / `require_classes` / class map
- `ID` → `when` `id` / `ids`, or `exclude_ids` / `include_ids`
- `MenuID` → `when` `menu_id` / `menu_ids`, or `exclude_menu_ids`
- `Pos` → `when` `x,y,w,h` / `pos` (top-level `positions` is reference only)

#### Examples

Keep `"File": "文件"` for the common case. Use an object only for clashes.

Same translation, skip some places:

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

- **Play**: buttons and menus become 「播放」; list cells, edit boxes, and control 2054 stay `Play`.
- **Track**: translate only on `Button` / `Static`; a list `Track` is left alone.
- **Title**: default is no translation; only header control `SysHeader32` becomes 「标题」. `translate("Title")` has no site, so it also stays English.
- **Preferences**: translate except for one menu ID.

Use `when` when the **same source needs different translations**:

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

- A list-cell `View` is left in English (usually track/column data).
- Control 105 becomes 「视图」.
- Buttons and static text become 「查看」.
- Menu 40020 becomes 「查看」.
- Anything else that passes uses `__default__` 「查看」.
- JS / C++ with no site use 「查看」 only.

JS panels need `Translate("Play", window.ID)` or `Translate("Title", "SysHeader32")` for these filters. `gr.GdiDrawText` goes through system hooks and gets the current HWND automatically.

## Harvesting missing strings

1. In foobar: **View → Dynamic Multilingual Engine → Developer Mode**.
2. Open the windows or menus that are still in English.
3. Open **Manage packs** (menu or Preferences): edit the pack on the left, handle `harvest.txt` on the right, or translate sources online. Changing packs in that window is for editing only and does not switch foobar2000's target language.
4. Or open `foo-lang/harvest.txt` and merge new lines into the JSON (key = source, value = translation or rule object). `# [Class: ...]` records the control so you can fill `when` / `exclude_*`.
5. Pick the language again, or restart foobar.

The online engine, AppID, and secret live under **Preferences → Display → Dynamic Multilingual Engine**. Public Google may rate-limit; official credentials are more reliable.

Keep the same key set across packs. The shipped packs follow the keys in `zh-CN.json`.

Upgrading the component does not change files you already edited. To restore a shipped seed: back it up, delete that json under `foo-lang`, and restart.
