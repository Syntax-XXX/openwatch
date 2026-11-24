/* Einstellungen: Schalterliste (Wisch hoch/runter scrollt), Tipp schaltet um; speichert in OW_SETTINGS */
#include "../widgets/owapp.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
typedef struct { const char *label, *icon; int kind; } Row;   /* kind: 0 wifi 1 bt 2 dnd 3 aod 4 plane 5 brightness */
static const Row rows[] = { {"WLAN", "wifi", 0}, {"Bluetooth", "bluetooth", 1}, {"Nicht stören", "moon", 2}, {"Always-on", "face", 3}, {"Flugmodus", "plane", 4}, {"Helligkeit", "sun", 5} };
#define N ((int)(sizeof rows / sizeof rows[0]))
static bool *flag(OwState *s, int k) { switch (k) { case 0: return &s->wifi; case 1: return &s->bluetooth; case 2: return &s->dnd; case 3: return &s->always_on; case 4: return &s->airplane; } return NULL; }
#define ROW_H 64
#define ROW_PITCH 72
static void draw(OwApp *a, cairo_t *cr) {
    OwState *s = &a->st;
    cairo_save(cr); cairo_rectangle(cr, 0, 0, 360, 360); cairo_clip(cr);
    for (int i = 0; i < N; i++) {
        double y = 96 + i * ROW_PITCH - a->scroll, sc, al; ow_fisheye(y + ROW_H / 2.0, &sc, &al);
        if (al < 0.06) continue;
        cairo_save(cr); cairo_translate(cr, 180, y + ROW_H / 2.0); cairo_scale(cr, sc, sc); cairo_translate(cr, -180, -(y + ROW_H / 2.0));
        ow_round_rect(cr, 34, y, 292, ROW_H, ROW_H / 2.0); ow_set(cr, OW_CARD, al); cairo_fill(cr);
        bool on = rows[i].kind == 5 ? true : *flag(s, rows[i].kind);
        ow_icon(cr, rows[i].icon, 70, y + ROW_H / 2.0, 26, on ? OW_ACCENT : OW_DIM);
        ow_text(cr, rows[i].label, 98, y + 38, 18, OW_FG, -1, false);
        if (rows[i].kind == 5) { char b[16]; snprintf(b, sizeof b, "%d%%", s->brightness); ow_text(cr, b, 308, y + 38, 17, OW_ACCENT, 1, true); }
        else { ow_pill(cr, 254, y + 20, 54, 24, on ? OW_ACCENT : OW_DIM, on ? 1 : 0.4);
               cairo_new_path(cr); cairo_arc(cr, on ? 296 : 266, y + 32, 9, 0, 2 * M_PI); ow_set(cr, OW_FG, 1); cairo_fill(cr); }
        cairo_restore(cr);
    }
    cairo_restore(cr);
    ow_text(cr, "Einstellungen", 180, 54, 16, OW_DIM, 0, false);
}
static bool gesture(OwApp *a, OwGesture g, int x, int y) {
    (void)x; OwState *s = &a->st;
    if (g == OW_G_SWIPE_UP) { a->scroll = fmin(a->scroll + 100, N * ROW_PITCH - 200); return true; }
    if (g == OW_G_SWIPE_DOWN) { a->scroll = fmax(0, a->scroll - 100); return true; }
    if (g == OW_G_TAP) for (int i = 0; i < N; i++) { double yy = 96 + i * ROW_PITCH - a->scroll; if (y >= yy && y < yy + ROW_H && y > 78 && y < 322) {
        if (rows[i].kind == 5) s->brightness = s->brightness >= 100 ? 20 : s->brightness + 20; else { bool *f = flag(s, rows[i].kind); *f = !*f; }
        ow_state_save_toggles(s, NULL); return true; } }
    return false;
}
int main(void) { OwApp a = { .name = "Einstellungen", .wm_class = "ow-settings", .tick_ms = 0, .draw = draw, .gesture = gesture }; return ow_app_run(&a); }
