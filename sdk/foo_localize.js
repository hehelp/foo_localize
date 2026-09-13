// foo_localize.js — JScript Panel 3 / JSplitter / SMP sample.
// Paste into the panel script. Call COM methods with parentheses.
// Startup writes ok/FAIL lines to the console so host mismatches show up immediately.
// Docs: https://github.com/hehelp/foo_localize/blob/main/docs/api.md

function RGB(r, g, b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

function log(msg) {
    try {
        console.log(msg);
    } catch (ignored) {}
}

function check(name, ok, extra) {
    log((ok ? "ok  " : "FAIL") + " " + name + (extra ? " " + extra : ""));
}

var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {
    log("FooLocalize FAIL: " + e.message);
}

if (engine) {
    try {
        var lang = engine.GetLanguage();
        check("GetLanguage", typeof lang === "string" && lang.length > 0, lang);
        check("IsEnabled", engine.IsEnabled() === true || engine.IsEnabled() === false, String(engine.IsEnabled()));
        check("Translate", typeof engine.Translate("Play") === "string", engine.Translate("Play"));
        check("Translate class", typeof engine.Translate("Title", "SysHeader32") === "string");
        check("Translate hwnd", typeof engine.Translate("Play", window.ID) === "string");
        check("SetPanelType", engine.SetPanelType("playlist") === true);
        check("SetPanelType bad", engine.SetPanelType("nope") === false);
        check("SetContextType", engine.SetContextType("menu") === true);
        check("ClearContextType", engine.ClearContextType("menu") === true);
        check("ClearContextType()", engine.ClearContextType() === true);
        engine.Skip();
        check("Skip Translate", engine.Translate("Play") === "Play");
        engine.Continue();
        check("Continue", engine.Translate("Play") !== "");
        check("HasTranslation", engine.HasTranslation("Play") === true || engine.HasTranslation("Play") === false);
        check("HasTranslation lang", engine.HasTranslation("Play", lang) === true || engine.HasTranslation("Play", lang) === false);
        var key = "FooLocalizeJsTest";
        engine.ModifyTranslation(key, "test", lang);
        check("ModifyTranslation", engine.HasTranslation(key, lang) === true);
    } catch (e) {
        log("FooLocalize FAIL: " + e.message);
        try {
            engine.Continue();
        } catch (ignored) {}
    }
}

var g_jsp3 = typeof gdi === "undefined" || !gdi.Font;
var g_font = g_jsp3
    ? JSON.stringify({Name: "Segoe UI", Size: 16})
    : gdi.Font("Segoe UI", 16, 0);
var g_font_hi = g_jsp3
    ? JSON.stringify({Name: "Segoe UI", Size: 22, Weight: 700})
    : gdi.Font("Segoe UI", 22, 1);

function _(text) {
    return engine ? engine.Translate(text) : text;
}

function draw_text(gr, text, font, color, x, y, w, h) {
    if (g_jsp3) {
        gr.WriteText(text, font, color, x, y, w, h);
        return;
    }
    gr.GdiDrawText(text, font, color, x, y, w, h, 0);
}

function on_paint(gr) {
    var w = window.Width;
    var h = window.Height;
    if (g_jsp3) {
        gr.Clear(RGB(20, 20, 26));
    } else {
        gr.FillSolidRect(0, 0, w, h, RGB(20, 20, 26));
    }

    if (!engine || !engine.IsEnabled()) {
        draw_text(gr, "Play", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
        return;
    }

    engine.SetPanelType("playlist");
    draw_text(gr, "playlist title", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
    draw_text(gr, _("Settings"), g_font, RGB(180, 180, 190), 10, 40, w - 20, 24);
    draw_text(gr, _("Play"), g_font_hi, RGB(255, 220, 80), 10, 70, w - 20, 28);
}

function on_mouse_lbtn_up(x, y) {
    if (!engine) {
        return;
    }
    if (!engine.HasTranslation("Settings")) {
        engine.AddTranslation("Settings", "Shezhi");
    }
    engine.ModifyTranslation("Settings", "Shezhi", "zh-CN");
    window.Repaint();
}
