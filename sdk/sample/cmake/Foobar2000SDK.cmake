# Minimal foobar2000 2.x SDK static libs for this sample (Windows).
# shared is compiled in because many SDK trees ship no prebuilt shared-x64.lib.

set(FB2K_SDK_ROOT "${FOOBAR_SDK_ROOT}")
set(FB2K_SDK_FB2K "${FB2K_SDK_ROOT}/foobar2000")

function(fb2k_apply_common_flags target)
    target_compile_definitions(${target} PRIVATE
        UNICODE
        _UNICODE
        WIN32
        _WINDOWS
        _CRT_SECURE_NO_WARNINGS
        _SCL_SECURE_NO_WARNINGS
        NOMINMAX
        SHARED_EXPORTS
    )
    target_compile_options(${target} PRIVATE
        /W3
        /fp:fast
        /utf-8
        /Zc:__cplusplus
        /permissive-
    )
    target_include_directories(${target} PUBLIC
        "${FB2K_SDK_ROOT}"
        "${FB2K_SDK_FB2K}"
    )
    set_property(TARGET ${target} PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endfunction()

file(GLOB PFC_SOURCES CONFIGURE_DEPENDS "${FB2K_SDK_ROOT}/pfc/*.cpp")
list(FILTER PFC_SOURCES EXCLUDE REGEX "(nix-objects|synchro_nix)\\.cpp$")

add_library(fb2k_pfc STATIC ${PFC_SOURCES})
fb2k_apply_common_flags(fb2k_pfc)
target_include_directories(fb2k_pfc PUBLIC "${FB2K_SDK_ROOT}/pfc")

set(FB2K_SHARED_SOURCES
    "${FB2K_SDK_FB2K}/shared/audio_math.cpp"
    "${FB2K_SDK_FB2K}/shared/crash_info.cpp"
    "${FB2K_SDK_FB2K}/shared/filedialogs.cpp"
    "${FB2K_SDK_FB2K}/shared/filedialogs_vista.cpp"
    "${FB2K_SDK_FB2K}/shared/font_description.cpp"
    "${FB2K_SDK_FB2K}/shared/minidump.cpp"
    "${FB2K_SDK_FB2K}/shared/modal_dialog.cpp"
    "${FB2K_SDK_FB2K}/shared/systray.cpp"
    "${FB2K_SDK_FB2K}/shared/text_drawing.cpp"
    "${FB2K_SDK_FB2K}/shared/utf8.cpp"
    "${FB2K_SDK_FB2K}/shared/utf8api.cpp"
    "${FB2K_SDK_FB2K}/shared/Utility.cpp"
)
add_library(fb2k_shared STATIC ${FB2K_SHARED_SOURCES})
fb2k_apply_common_flags(fb2k_shared)
target_link_libraries(fb2k_shared PUBLIC fb2k_pfc)
target_link_libraries(fb2k_shared PRIVATE comctl32 uxtheme imagehlp dbghelp)

file(GLOB FB2K_SDK_SOURCES CONFIGURE_DEPENDS "${FB2K_SDK_FB2K}/SDK/*.cpp")
add_library(fb2k_sdk STATIC ${FB2K_SDK_SOURCES})
fb2k_apply_common_flags(fb2k_sdk)
target_include_directories(fb2k_sdk PUBLIC "${FB2K_SDK_FB2K}/SDK")
target_link_libraries(fb2k_sdk PUBLIC fb2k_pfc fb2k_shared)

add_library(fb2k_component_client STATIC
    "${FB2K_SDK_FB2K}/foobar2000_component_client/component_client.cpp"
)
fb2k_apply_common_flags(fb2k_component_client)
target_link_libraries(fb2k_component_client PUBLIC fb2k_sdk)
