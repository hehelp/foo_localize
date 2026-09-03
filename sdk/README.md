# foo_localize SDK

给其他 foobar2000 2.0+ 组件用的接口。只要这一份头文件，**不要**链接 foo_localize 的 lib 或 DLL。

| 文件 | 说明 |
| --- | --- |
| [`foo_localize_api.h`](foo_localize_api.h) | 公开接口：`localize_api`、`localize_notify` |
| [`sample/`](sample/) | 可编译的示例插件，演示全部调用 |

运行时若用户没装 foo_localize，`localize_api::tryGet` 返回 false，调用方应继续用原文。

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

宽字符原文用 `translate_w`，译文仍是 UTF-8。

语言包 id 与 `foo-lang` 文件名一致，例如 `zh-CN`。

```cpp
localize_api::ptr api;
if (!localize_api::tryGet(api)) {
    return;
}
pfc::string8 current;
api->get_current_language(current);

pfc::string_list_impl langs;
api->get_available_languages(langs);

api->set_language("zh-CN"); // 写入用户配置
```

`set_enabled` / `set_language` 会写入 foobar 的 `cfg_var`。只应在用户明确操作时调用，不要在后台改语言。

## 监听语言变化

```cpp
class my_localize_notify : public localize_notify {
public:
    void on_language_changed(const char* new_language) override {
        (void)new_language; // 只在本次调用内有效，要保存请立刻拷走
        refresh_my_ui();    // 不要在这里再调 set_language
    }
};

FB2K_SERVICE_FACTORY(my_localize_notify);
```

菜单「查看 → 语言」、首选项、以及其他组件调用 `set_language` 成功且语言确实变了时，都会收到广播。

## GUID

| 接口 | GUID |
| --- | --- |
| `localize_api` | `{5E8A1C3B-7042-4D16-9F28-A6B3D04E8C17}` |
| `localize_notify` | `{C4D29B70-1E58-4A93-86F0-2B7C5D9A4138}` |

完整示例与编译说明见 [`sample/README.md`](sample/README.md)。
