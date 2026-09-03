// foo_localize_sample — example foobar2000 component that calls foo_localize.
// Copy ../foo_localize_api.h next to your sources (or add sdk/ to include path).
// Do not link any foo_localize library. If the engine is not installed,
// localize_api::tryGet returns false and this component keeps working.

#include <foobar2000.h>
#include "foo_localize_api.h"

#include <cstdint>
#include <string>
#include <vector>

#ifndef FOO_LOCALIZE_SAMPLE_VERSION
#define FOO_LOCALIZE_SAMPLE_VERSION "1.0.0"
#endif

namespace {

// {3A91C6E0-8D14-4B27-9F50-2C7E1A4D6839}
const GUID guid_menu_group = {
    0x3a91c6e0, 0x8d14, 0x4b27, {0x9f, 0x50, 0x2c, 0x7e, 0x1a, 0x4d, 0x68, 0x39}
};
// {4B02D7F1-9E25-4C38-A061-3D8F2B5E794A}
const GUID guid_cmd_status = {
    0x4b02d7f1, 0x9e25, 0x4c38, {0xa0, 0x61, 0x3d, 0x8f, 0x2b, 0x5e, 0x79, 0x4a}
};
// {5C13E802-0F36-4D49-B172-4E9A3C6F805B}
const GUID guid_cmd_translate = {
    0x5c13e802, 0x0f36, 0x4d49, {0xb1, 0x72, 0x4e, 0x9a, 0x3c, 0x6f, 0x80, 0x5b}
};
// {6D24F913-1047-4E5A-C283-5F0B4D70916C}
const GUID guid_cmd_dump = {
    0x6d24f913, 0x1047, 0x4e5a, {0xc2, 0x83, 0x5f, 0x0b, 0x4d, 0x70, 0x91, 0x6c}
};
// {7E35A024-2158-4F6B-D394-601C5E81A27D}
const GUID guid_cmd_toggle = {
    0x7e35a024, 0x2158, 0x4f6b, {0xd3, 0x94, 0x60, 0x1c, 0x5e, 0x81, 0xa2, 0x7d}
};
// {8F46B135-3269-407C-E4A5-712D6F92B38E}
const GUID guid_cmd_language = {
    0x8f46b135, 0x3269, 0x407c, {0xe4, 0xa5, 0x71, 0x2d, 0x6f, 0x92, 0xb3, 0x8e}
};
// {9057C246-437A-418D-F5B6-823E70A3C49F}
const GUID guid_cmd_language_empty = {
    0x9057c246, 0x437a, 0x418d, {0xf5, 0xb6, 0x82, 0x3e, 0x70, 0xa3, 0xc4, 0x9f}
};

const char* const k_title = "foo_localize API sample";
const char* const k_demo_words[] = {"File", "Preferences", "Play", "View"};

bool try_api(localize_api::ptr& api) {
    return localize_api::tryGet(api);
}

void translate_or_copy(pfc::string_base& out, const char* english) {
    localize_api::ptr api;
    if (try_api(api) && api->translate(english, out)) {
        return;
    }
    out = english;
}

pfc::string8 current_language_id() {
    localize_api::ptr api;
    pfc::string8 lang;
    if (try_api(api)) {
        api->get_current_language(lang);
    }
    return lang;
}

GUID guid_from_pack_id(const char* id) {
    GUID guid = guid_cmd_language;
    std::uint32_t h1 = 2166136261u;
    std::uint32_t h2 = 0x811c9dc5u;
    if (id != nullptr) {
        for (const unsigned char* p = reinterpret_cast<const unsigned char*>(id); *p != 0; ++p) {
            h1 ^= *p;
            h1 *= 16777619u;
            h2 = h2 * 33u + *p;
        }
    }
    guid.Data1 ^= h1;
    guid.Data2 ^= static_cast<std::uint16_t>(h2);
    guid.Data3 ^= static_cast<std::uint16_t>(h1 >> 16);
    for (int i = 0; i < 8; ++i) {
        guid.Data4[i] ^= static_cast<std::uint8_t>((h1 >> ((i * 3) % 24)) ^ (h2 >> i));
    }
    return guid;
}

class SampleVersion : public componentversion {
public:
    void get_file_name(pfc::string_base& out) override {
        out = core_api::get_my_file_name();
    }
    void get_component_name(pfc::string_base& out) override {
        out = k_title;
    }
    void get_component_version(pfc::string_base& out) override {
        out = FOO_LOCALIZE_SAMPLE_VERSION;
    }
    void get_about_message(pfc::string_base& out) override {
        out = "Example component that talks to foo_localize through foo_localize_api.h.\n";
        out += "No link against foo_localize. If the engine is missing, menus still work.\n\n";
        localize_api::ptr api;
        if (try_api(api)) {
            pfc::string8 lang;
            api->get_current_language(lang);
            out += "foo_localize loaded. language=";
            out += lang;
            out += api->is_enabled() ? " enabled=yes" : " enabled=no";
        } else {
            out += "foo_localize is not installed.";
        }
    }
};

FB2K_SERVICE_FACTORY(SampleVersion);

class SampleInit : public initquit {
public:
    void on_init() override {
        localize_api::ptr api;
        if (try_api(api)) {
            pfc::string8 lang;
            api->get_current_language(lang);
            FB2K_console_formatter() << k_title << ": api ready, language=" << lang
                                     << (api->is_enabled() ? ", enabled" : ", disabled");
        } else {
            FB2K_console_formatter() << k_title << ": foo_localize not found (tryGet == false)";
        }
    }
    void on_quit() override {}
};

FB2K_SERVICE_FACTORY(SampleInit);

// Language-change callback. Pointer is valid only during this call.
class SampleNotify : public localize_notify {
public:
    void on_language_changed(const char* new_language) override {
        FB2K_console_formatter() << k_title << ": on_language_changed("
                                 << (new_language != nullptr ? new_language : "") << ")";
        // Redraw your own UI here. Do not call set_language() from this callback.
    }
};

FB2K_SERVICE_FACTORY(SampleNotify);

class SampleMenuGroup : public mainmenu_group_popup {
public:
    GUID get_guid() override { return guid_menu_group; }
    GUID get_parent() override { return mainmenu_groups::view; }
    t_uint32 get_sort_priority() override { return mainmenu_commands::sort_priority_dontcare; }
    void get_display_string(pfc::string_base& out) override { out = k_title; }
};

FB2K_SERVICE_FACTORY(SampleMenuGroup);

class PackCommand : public mainmenu_node_command {
public:
    explicit PackCommand(pfc::string8 id) : id_(std::move(id)) {}

    void get_display(pfc::string_base& text, t_uint32& flags) override {
        text = id_;
        flags = (current_language_id() == id_) ? mainmenu_commands::flag_radiochecked : 0;
    }

    void execute(service_ptr_t<service_base>) override {
        localize_api::ptr api;
        if (!try_api(api)) {
            popup_message::g_show("foo_localize is not installed.", k_title);
            return;
        }
        // User-initiated switch only. Writes cfg_var; other components get localize_notify.
        if (!api->set_language(id_.c_str())) {
            popup_message::g_show("Unknown language pack.", k_title);
        }
    }

    GUID get_guid() override { return guid_from_pack_id(id_.c_str()); }

    bool get_description(pfc::string_base& out) override {
        out = "Call localize_api::set_language()";
        return true;
    }

private:
    pfc::string8 id_;
};

class EmptyPackCommand : public mainmenu_node_command {
public:
    void get_display(pfc::string_base& text, t_uint32& flags) override {
        text = "(no language packs)";
        flags = mainmenu_commands::flag_disabled;
    }
    void execute(service_ptr_t<service_base>) override {}
    GUID get_guid() override { return guid_cmd_language_empty; }
};

class LanguageGroup : public mainmenu_node_group {
public:
    LanguageGroup() {
        localize_api::ptr api;
        if (try_api(api)) {
            pfc::string_list_impl langs;
            api->get_available_languages(langs);
            children_.reserve(langs.get_count());
            for (t_size i = 0; i < langs.get_count(); ++i) {
                children_.push_back(fb2k::service_new<PackCommand>(pfc::string8(langs[i])));
            }
        }
        if (children_.empty()) {
            children_.push_back(fb2k::service_new<EmptyPackCommand>());
        }
    }

    void get_display(pfc::string_base& text, t_uint32& flags) override {
        text = "set_language";
        flags = 0;
    }
    t_size get_children_count() override { return children_.size(); }
    mainmenu_node::ptr get_child(t_size index) override {
        if (index >= children_.size()) {
            throw pfc::exception_invalid_params();
        }
        return children_[index];
    }

private:
    std::vector<mainmenu_node::ptr> children_;
};

class SampleMenu : public mainmenu_commands_v2 {
public:
    enum {
        cmd_status = 0,
        cmd_translate,
        cmd_dump,
        cmd_toggle,
        cmd_language,
        cmd_total
    };

    t_uint32 get_command_count() override { return cmd_total; }

    GUID get_command(t_uint32 index) override {
        switch (index) {
        case cmd_status:
            return guid_cmd_status;
        case cmd_translate:
            return guid_cmd_translate;
        case cmd_dump:
            return guid_cmd_dump;
        case cmd_toggle:
            return guid_cmd_toggle;
        case cmd_language:
            return guid_cmd_language;
        default:
            uBugCheck();
        }
    }

    void get_name(t_uint32 index, pfc::string_base& out) override {
        switch (index) {
        case cmd_status:
            out = "Status";
            break;
        case cmd_translate:
            out = "translate(\"Preferences\")";
            break;
        case cmd_dump:
            out = "Dump API state";
            break;
        case cmd_toggle:
            out = "set_enabled";
            break;
        case cmd_language:
            out = "set_language";
            break;
        default:
            break;
        }
    }

    bool get_description(t_uint32 index, pfc::string_base& out) override {
        switch (index) {
        case cmd_status:
            out = "Show tryGet / is_enabled / get_current_language";
            return true;
        case cmd_translate:
            out = "Call localize_api::translate()";
            return true;
        case cmd_dump:
            out = "Print get_available_languages() to console and a popup";
            return true;
        case cmd_toggle:
            out = "Call localize_api::set_enabled()";
            return true;
        case cmd_language:
            out = "Call localize_api::set_language()";
            return true;
        default:
            return false;
        }
    }

    GUID get_parent() override { return guid_menu_group; }

    bool get_display(t_uint32 index, pfc::string_base& text, t_uint32& flags) override {
        flags = 0;
        localize_api::ptr api;
        const bool have = try_api(api);

        if (index == cmd_status) {
            if (!have) {
                text = "foo_localize: not installed";
            } else {
                pfc::string8 lang;
                api->get_current_language(lang);
                text = "foo_localize: ";
                text += lang.get_length() > 0 ? lang.c_str() : "(none)";
                text += api->is_enabled() ? "  [on]" : "  [off]";
            }
            flags = mainmenu_commands::flag_disabled;
            return true;
        }

        get_name(index, text);
        if (!have && index != cmd_status) {
            flags = mainmenu_commands::flag_disabled;
        }
        if (have && index == cmd_toggle && api->is_enabled()) {
            flags |= mainmenu_commands::flag_checked;
        }
        return true;
    }

    void execute(t_uint32 index, service_ptr_t<service_base>) override {
        localize_api::ptr api;
        if (!try_api(api)) {
            popup_message::g_show("foo_localize is not installed.\nlocalize_api::tryGet() returned false.", k_title);
            return;
        }

        if (index == cmd_translate) {
            pfc::string8 body;
            for (const char* word : k_demo_words) {
                pfc::string8 translated;
                body += word;
                body += "  ->  ";
                if (api->translate(word, translated)) {
                    body += translated;
                } else {
                    body += "(miss)";
                }
                body += "\n";
            }
            pfc::string8 wide_hit;
            const bool wide_ok = api->translate_w(L"Preferences", wide_hit);
            body += "translate_w(L\"Preferences\")  ->  ";
            body += wide_ok ? wide_hit.c_str() : "(miss)";
            popup_message::g_show(body, k_title);
            return;
        }

        if (index == cmd_dump) {
            pfc::string8 lang;
            api->get_current_language(lang);
            pfc::string_list_impl langs;
            api->get_available_languages(langs);

            pfc::string8 body;
            body += "is_enabled = ";
            body += api->is_enabled() ? "true" : "false";
            body += "\nget_current_language = ";
            body += lang;
            body += "\nget_available_languages:\n";
            for (t_size i = 0; i < langs.get_count(); ++i) {
                body += "  ";
                body += langs[i];
                body += "\n";
                FB2K_console_formatter() << k_title << ": pack " << langs[i];
            }
            popup_message::g_show(body, k_title);
            return;
        }

        if (index == cmd_toggle) {
            api->set_enabled(!api->is_enabled());
        }
    }

    bool is_command_dynamic(t_uint32 index) override { return index == cmd_language; }

    mainmenu_node::ptr dynamic_instantiate(t_uint32) override {
        return fb2k::service_new<LanguageGroup>();
    }
};

static mainmenu_commands_factory_t<SampleMenu> g_sample_menu;

} // namespace

VALIDATE_COMPONENT_FILENAME("foo_localize_sample.dll");

FOOBAR2000_IMPLEMENT_CFG_VAR_DOWNGRADE;
