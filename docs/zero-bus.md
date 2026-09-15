# Zero Bus：`plugin.localize`

English: [zero-bus.en.md](zero-bus.en.md)。

Windows 与 macOS 都提供。依赖已安装并启用的 [`foo_zero_bus`](https://github.com/hehelp/foo_zero_bus)。未安装时本组件照常工作，只是没有这条总线服务。

服务名：`plugin.localize`。总线只路由信封，**不解析**业务 JSON。`payload` 必须是字符串；WebSocket 要做双层 `JSON.stringify`（先业务对象，再整封信封）。默认 `ws://127.0.0.1:17890`。

COM `FooLocalize.Engine` 仍仅 Windows。本服务**没有** `Skip` / `SetPanelType` / 按 hwnd 换字闸。

## 请求

| cmd | 参数 | 成功回复 |
| --- | --- | --- |
| `is_enabled` | 无 | `{"ok":true,"enabled":true}` |
| `set_enabled` | `enabled`（bool） | `{"ok":true}` |
| `get_language` | 无 | `{"ok":true,"lang":"zh-CN"}` |
| `list_languages` | 无 | `{"ok":true,"languages":["zh-CN",...]}` |
| `set_language` | `lang` | `{"ok":true}` |
| `translate` | `text` | 命中 `{"ok":true,"hit":true,"text":"..."}`；未命中 `{"ok":true,"hit":false}` |
| `translate_site` | `text`，可选 `class` / `id` / `menu_id` / `x` `y` `w` `h` / `hwnd`（Windows） | 同上 |
| `has_translation` | `text`，可选 `lang` | `{"ok":true,"found":true}` |
| `add_translation` | `text`、`translated`，可选 `lang` | `{"ok":true}` |
| `modify_translation` | 同上 | `{"ok":true}` |

失败：`{"ok":false,"error":"..."}`。`translate` 未命中**不**回原文（与 C++ `localize_api` 一致，与 COM `Translate` 不同）。

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

## 事件

语言实际变更时广播：

```json
{"event":"language_changed","lang":"zh-CN"}
```

`sender` 与 `receiver` 都是 `plugin.localize`。
