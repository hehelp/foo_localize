# Language pack guide

A language pack is UTF-8 JSON with a flat key/value map. The key is the English UI string shown by foobar2000; the value is the translation.

On first launch the built-in seeds are written to:

- Windows: `%APPDATA%\foobar2000-v2\foo-lang\`
- macOS: `~/Library/foobar2000-v2/foo-lang/`

**A file is written only if it does not already exist.** Existing JSON is never overwritten. [`dict/`](../dict/) in this repo matches the built-in seeds.

The file name without `.json` is the pack id (for example `zh-CN`). The name in the Language menu comes from `@name` in the JSON.

## Format

```json
{
  "@name": "English (template)",
  "File": "",
  "Preferences": "",
  "volume": ""
}
```

- `@name` appears only in the language menu and is not substituted into the UI.
- An empty value means “leave the original text”. Do not use empty to mean “hide”.
- Menu accelerators sit after `\t` in the source (e.g. `Play\tSpace`). Translate only the visible part before `\t`.
- Do not translate product names, paths, URLs, `foo_*` component ids, track titles, font names, Title Formatting, or config scripts.

To start a new language, copy `en-US-template.json` (all values empty), change `@name` and the file name, then fill in translations.

## Harvesting missing strings

1. In foobar: **View → Dynamic Multilingual Engine → Developer Mode**.
2. Open the windows or menus that are still in English.
3. Open `foo-lang/harvest.txt` and merge new lines into the JSON (key = source, value = translation).
4. Pick the language again, or restart foobar.

Keep the same key set across packs. The shipped packs follow the keys in `zh-CN.json`.

Upgrading the component does not change files you already edited. To restore a shipped seed: back it up, delete that json under `foo-lang`, and restart.
