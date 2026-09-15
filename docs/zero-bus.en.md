# Zero Bus: `plugin.localize`

中文：[zero-bus.md](zero-bus.md).

Available on Windows and macOS. Requires [`foo_zero_bus`](https://github.com/hehelp/foo_zero_bus) installed and running. If it is missing, this component still works; the bus service is simply not registered.

Service name: `plugin.localize`. The bus routes envelopes only and **never** parses the business JSON. `payload` must be a string. WebSocket clients must double-`JSON.stringify` (business object, then the envelope). Default `ws://127.0.0.1:17890`.

COM `FooLocalize.Engine` remains Windows-only. This service does **not** expose `Skip` / `SetPanelType` / per-hwnd replacement gates.

## Requests

| cmd | Arguments | Success |
| --- | --- | --- |
| `is_enabled` | none | `{"ok":true,"enabled":true}` |
| `set_enabled` | `enabled` (bool) | `{"ok":true}` |
| `get_language` | none | `{"ok":true,"lang":"zh-CN"}` |
| `list_languages` | none | `{"ok":true,"languages":["zh-CN",...]}` |
| `set_language` | `lang` | `{"ok":true}` |
| `translate` | `text` | hit `{"ok":true,"hit":true,"text":"..."}`; miss `{"ok":true,"hit":false}` |
| `translate_site` | `text`, optional `class` / `id` / `menu_id` / `x` `y` `w` `h` / `hwnd` (Windows) | same |
| `has_translation` | `text`, optional `lang` | `{"ok":true,"found":true}` |
| `add_translation` | `text`, `translated`, optional `lang` | `{"ok":true}` |
| `modify_translation` | same | `{"ok":true}` |

Failure: `{"ok":false,"error":"..."}`. A miss does **not** return the source text (same as C++ `localize_api`, unlike COM `Translate`).

```js
const envelope = {
  sender: "",
  receiver: "plugin.localize",
  type: 1,
  msg_id: "req_1",
  correlation_id: "",
  payload: JSON.stringify({ cmd: "translate", text: "Play" }),
};
socket.send(JSON.stringify(envelope));
```

## Events

Broadcast when the language pack actually changes:

```json
{"event":"language_changed","lang":"zh-CN"}
```

Both `sender` and `receiver` are `plugin.localize`.
