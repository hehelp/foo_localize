// foo_localize.js — JScript Panel 3 sample (same drawing style as Klyrics).
// Paste into a JScript Panel 3.4+ script. Do not use GdiDrawText / DrawString / gdi.Font.
// Docs: https://github.com/hehelp/foo_localize/blob/main/docs/api.md

function RGB(r, g, b) {
    return 0xff000000 | (r << 16) | (g << 8) | b;
}

var engine = null;
try {
    engine = new ActiveXObject("FooLocalize.Engine");
} catch (e) {}

var g_font = JSON.stringify({Name: "Segoe UI", Size: 16});
var g_font_hi = JSON.stringify({Name: "Segoe UI", Size: 22, Weight: 700});

function _(text) {
    return engine ? engine.Translate(text) : text;
}

function on_paint(gr) {
    var w = window.Width;
    gr.Clear(RGB(20, 20, 26));

    if (!engine || !engine.IsEnabled) {
        gr.WriteText("Play", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
        return;
    }

    engine.SetPanelType("playlist");
    gr.WriteText("playlist title", g_font, RGB(160, 160, 160), 10, 10, w - 20, 24);
    gr.WriteText(_("Settings"), g_font, RGB(180, 180, 190), 10, 40, w - 20, 24);
    gr.WriteText(_("Play"), g_font_hi, RGB(255, 220, 80), 10, 70, w - 20, 28);
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
