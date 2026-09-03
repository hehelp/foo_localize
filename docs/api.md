# 组件 API（foo_localize）

接口头文件和可编译示例都在 [`../sdk/`](../sdk/README.md)。

- 头文件：[`../sdk/foo_localize_api.h`](../sdk/foo_localize_api.h)
- 示例插件：[`../sdk/sample/`](../sdk/sample/README.md)

把 `foo_localize_api.h` 拷进你的工程即可，**不要**链接任何 foo_localize 库。运行时若没装引擎，`localize_api::tryGet` 返回 false，调用方应继续用原文。

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
