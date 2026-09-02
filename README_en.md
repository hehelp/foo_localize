# Dynamic Multilingual Engine (foo_localize)

A runtime localization component for foobar2000. It does not patch the official binaries. UI English is replaced from external JSON language packs. Swap the pack to switch Simplified Chinese, Traditional Chinese, Japanese, Russian, Spanish, German, French, and others. Chinese display name **动态多语引擎**.

Component version: `1.0.0`. 中文：[README.md](README.md).

This repository hosts **release notes** and open language-pack JSON. The component source is not published. Get the installer from the repo **[Releases](https://github.com/hehelp/foo_localize/releases)** page.

## Updates

### 1.0.0 (2026-09-02)

- First release: Windows 32 / 64-bit and macOS
- Main menus, context menus, Preferences, dialogs, and window titles follow the language pack
- **View → Language** switches packs live; Developer Mode writes missing strings to `harvest.txt`
- Built-in Simplified / Traditional Chinese, Japanese, Russian, Spanish, German, French, and an English template; first launch writes them to `foo-lang` (existing files are never overwritten)

## Screenshots

### Windows

![Components](screenshot/win/install.png)

![View menu](screenshot/win/menu-item.png)

![Preferences](screenshot/win/setting.png)

### macOS

![Components](screenshot/macOS/install.png)

![View menu](screenshot/macOS/view-item.png)

![Preferences](screenshot/macOS/setting.png)

## Supported players

| Item | Requirement |
| --- | --- |
| OS | Windows 10 / 11; macOS 11+ |
| Player | **foobar2000 2.0+** (32-bit or 64-bit) |
| Not supported | foobar2000 1.x; Windows components cannot be loaded on Mac and vice versa |

32-bit and 64-bit are two different DLLs. Do not mix them.

| Player | Component file | Install folder |
| --- | --- | --- |
| foobar2000 2.x **32-bit** | `foo_localize.dll` | `%APPDATA%\foobar2000-v2\user-components\foo_localize\` |
| foobar2000 2.x **64-bit** | `foo_localize.dll` | `%APPDATA%\foobar2000-v2\user-components-x64\foo_localize\` |
| foobar2000 **Mac** | `foo_localize.component` | `~/Library/foobar2000-v2/user-components/` |

Language packs stay in the user profile, not the program folder:

| OS | Language-pack folder |
| --- | --- |
| Windows | `%APPDATA%\foobar2000-v2\foo-lang\` |
| macOS | `~/Library/foobar2000-v2/foo-lang\` |

## Install

1. Open **[Releases](https://github.com/hehelp/foo_localize/releases)** and download **`foo_localize-1.0.0.fb2k-component`** (official foobar package: one zip with 32-bit, 64-bit, and macOS; Install picks the matching binary).
2. In foobar: **File → Preferences → Components → Install**, then pick that file.
3. Or copy the matching DLL / `.component` into the folder above and **fully quit, then reopen** foobar2000.

On first launch the built-in packs are written to `foo-lang` **only when that file is missing**. Existing JSON is never overwritten, so upgrading the component does not discard your edits.

A typical 64-bit Windows path:

```
C:\Users\<name>\AppData\Roaming\foobar2000-v2\user-components-x64\foo_localize\foo_localize.dll
C:\Users\<name>\AppData\Roaming\foobar2000-v2\foo-lang\
```

foobar cannot overwrite the DLL while it is running. Quit first.

## Usage

- **Switch language**: **View → Language**, or **File → Preferences → Display → Dynamic Multilingual Engine**.
- **Master switch / Developer Mode**: **View → Dynamic Multilingual Engine**.
- Developer Mode appends unmatched UI English to `foo-lang/harvest.txt`. Merge new keys into the JSON, then pick the language again or restart.

`en-US-template.json` has empty values, so selecting it shows the original English. Use it as a blank for a new pack.

Menus and dialogs refresh immediately after a switch. A few owner-drawn controls may need the window reopened.

## Language-pack rules

The English UI string is the key; the translation is the JSON value. An empty value leaves the original text on screen. Do not use an empty string to mean “hide”.

| Rule | Meaning |
| --- | --- |
| Exact match | Look up the on-screen source text; ASCII case can fold, and a leading `•` / surrounding spaces are stripped first |
| `@name` | Language-menu label only; not substituted into the UI |
| Menu accelerators | Text after `\t` is the shortcut; translate only the visible part before `\t` |
| Empty value | Do not translate |
| Do not translate | Track titles, file names, playlist names, paths, URLs, `foobar2000`, `foo_*`, `ReFacets`, `UPnP`, `FFmpeg`, font names, Title Formatting, config scripts |
| First-run seed | A built-in pack is written only if that file is **absent** from `foo-lang` |
| Upgrades | Existing JSON is kept; to restore a shipped pack, back it up, delete that file, and restart |
| Harvest | Developer Mode → `harvest.txt` → merge into JSON → pick the language again |

[`dict/`](dict/) in this repo matches the built-in seeds. Writing guide: [Language pack guide](docs/lang-pack.en.md).

Shipped files:

| File | Display name |
| --- | --- |
| `zh-CN.json` | 简体中文 |
| `zh-TW.json` | 繁體中文 |
| `ja-JP.json` | 日本語 |
| `ru-RU.json` | Русский |
| `es-ES.json` | Español |
| `de-DE.json` | Deutsch |
| `fr-FR.json` | Français |
| `en-US-template.json` | English (template) |

The Windows **Columns UI** status-bar volume cell currently draws `-3.00 dB`, not the word `volume`. That path is deferred.

## Preferences

**File → Preferences → Display → Dynamic Multilingual Engine** (Chinese UI: **动态多语引擎**)

Enable the engine, turn on Developer Mode, and choose a language pack. The View menu group follows the UI language: **动态多语引擎** in Chinese, **Dynamic Multilingual Engine** in English.
