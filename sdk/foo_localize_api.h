#pragma once

// Public foobar2000 service API for foo_localize.
// Other components: include this header, then
//   localize_api::ptr api;
//   if (localize_api::tryGet(api)) { ... }
// To hear language changes, implement localize_notify and register
//   FB2K_SERVICE_FACTORY(your_notify);
// GUIDs: {5E8A1C3B-7042-4D16-9F28-A6B3D04E8C17} / {C4D29B70-1E58-4A93-86F0-2B7C5D9A4138}
// Sample component: sdk/sample/

#if __has_include(<foobar2000.h>)
#include <foobar2000.h>
#else
#include <SDK/foobar2000.h>
#endif

class NOVTABLE localize_api : public service_base {
    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(localize_api);

public:
    virtual bool is_enabled() = 0;
    virtual void set_enabled(bool state) = 0;

    virtual void get_available_languages(pfc::string_list_impl& out_list) = 0;
    virtual void get_current_language(pfc::string_base& out_lang) = 0;

    // lang_name is a pack id such as "zh-CN". Persists cfg_var. Returns false if unknown.
    virtual bool set_language(const char* lang_name) = 0;

    // true and writes the translation when the dictionary hits; false leaves out_translated unused.
    virtual bool translate(const char* original_text, pfc::string_base& out_translated) = 0;
    virtual bool translate_w(const wchar_t* original_text, pfc::string_base& out_translated_utf8) = 0;

    static bool tryGet(ptr& out) { return enumerate().first(out); }
    static ptr tryGet() {
        ptr out;
        tryGet(out);
        return out;
    }
};

class NOVTABLE localize_notify : public service_base {
    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(localize_notify);

public:
    virtual void on_language_changed(const char* new_language) = 0;
};

// {5E8A1C3B-7042-4D16-9F28-A6B3D04E8C17}
inline const GUID localize_api::class_guid = {
    0x5e8a1c3b, 0x7042, 0x4d16, {0x9f, 0x28, 0xa6, 0xb3, 0xd0, 0x4e, 0x8c, 0x17}
};

// {C4D29B70-1E58-4A93-86F0-2B7C5D9A4138}
inline const GUID localize_notify::class_guid = {
    0xc4d29b70, 0x1e58, 0x4a93, {0x86, 0xf0, 0x2b, 0x7c, 0x5d, 0x9a, 0x41, 0x38}
};

inline const GUID guid_localize_api = localize_api::class_guid;
inline const GUID guid_localize_notify = localize_notify::class_guid;
