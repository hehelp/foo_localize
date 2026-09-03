# foo_localize 调用示例

示例组件 `foo_localize_sample` 演示如何通过 [`../foo_localize_api.h`](../foo_localize_api.h) 调用动态多语引擎。不链接 foo_localize。

装进 foobar2000 后，看 **查看 → foo_localize API sample**：

| 菜单 | 调用 |
| --- | --- |
| 状态行 | `tryGet` / `is_enabled` / `get_current_language` |
| `translate("Preferences")` | `translate`、`translate_w` |
| Dump API state | `get_available_languages` |
| `set_enabled` | 开关引擎（写入用户配置） |
| `set_language` | 子菜单列出语言包并切换 |

控制台会在启动和语言切换时打印日志（`localize_notify::on_language_changed`）。

没装 foo_localize 时菜单仍在，状态行显示 `not installed`，其它项禁用。

源码只有一个文件：[`src/foo_localize_sample.cpp`](src/foo_localize_sample.cpp)。可直接拷进你自己的组件工程。

## 编译（Windows）

需要 [foobar2000 SDK](https://www.foobar2000.org/SDK)（2.0+，含 `foobar2000/SDK/foobar2000.h`）和 CMake 3.24+、Visual Studio。

```bat
cd sdk\sample
cmake -S . -B build -DFOOBAR_SDK_ROOT=C:\path\to\foobar-sdk
cmake --build build --config Release
```

产物：`build\bin\Release\foo_localize_sample.dll`

拷到（先退出 foobar2000）：

- 64 位：`%APPDATA%\foobar2000-v2\user-components-x64\foo_localize_sample\`
- 32 位：用 Win32 工具链编，拷到 `user-components\foo_localize_sample\`

本机开发仓若 SDK 在 `foobar_localize/../foobar_lrc/foobar-sdk`，不传 `FOOBAR_SDK_ROOT` 也会自动找到。

## macOS

这份 CMake 只覆盖 Windows。同一个 `.cpp` 可以加进你现有的 Mac 组件 target，把头文件目录指到 `sdk/`。不要用这里的 `VALIDATE_COMPONENT_FILENAME("foo_localize_sample.dll")` 文件名去对 Mac bundle；Mac 上改成你的 `.component` 名，或去掉这行。
