/* Benachrichtigungen: scrollbare Kartenliste (Wisch hoch/runter), Tipp = Karte aufklappen */
#include "../widgets/owapp.h"
#include <math.h>
#include <stdio.h>
static int open_idx = -1;
static void draw(OwApp *a, cairo_t *cr) {
    OwState *s = &a->st; ow_text(cr, "Mitteilungen", 180, 54, 16, OW_DIM, 0, false);
    if (!s->notif_count) { ow_icon(cr, "bell", 180, 160, 44, OW_DIM); ow_text(cr, "Keine Mitteilungen", 180, 230, 18, OW_DIM, 0, false); return; }
    cairo_save(cr); cairo_rectangle(cr, 0, 0, 360, 360); cairo_clip(cr);
    double y = 84 - a->scroll;
    for (int i = 0; i < s->notif_count; i++) {
        OwNotif *n = &s->notif[i]; double h = (i == open_idx) ? 118 : 66, sc, al; ow_fisheye(y + h / 2, &sc, &al);
        if (al > 0.06) {
            cairo_save(cr); cairo_translate(cr, 180, y + h / 2); cairo_scale(cr, sc, sc); cairo_translate(cr, -180, -(y + h / 2));
            ow_round_rect(cr, 32, y, 296, h, 22); ow_set(cr, OW_CARD, al); cairo_fill(cr);
            ow_text(cr, n->app, 50, y + 22, 12, OW_ACCENT, -1, true); ow_text(cr, n->time, 310, y + 22, 12, OW_DIM, 1, false);
            ow_text_fit(cr, n->title, 50, y + 44, 258, 16, OW_FG, -1, true);
            if (i == open_idx) ow_text_fit(cr, n->text, 50, y + 70, 258, 14, OW_DIM, -1, false);
            else ow_text_fit(cr, n->text, 50, y + 61, 258, 12, OW_DIM, -1, false);
            cairo_restore(cr);
        }
        y += h + 8;
    }
    cairo_restore(cr);
}
static bool gesture(OwApp *a, OwGesture g, int x, int y) {
    OwState *s = &a->st; (void)x;
    if (g == OW_G_SWIPE_UP) { a->scroll += 90; return true; }
    if (g == OW_G_SWIPE_DOWN) { a->scroll = fmax(0, a->scroll - 90); return true; }
    if (g == OW_G_TAP) { double yy = 84 - a->scroll;
        for (int i = 0; i < s->notif_count; i++) { double h = (i == open_idx) ? 118 : 66; if (y >= yy && y < yy + h) { open_idx = (open_idx == i) ? -1 : i; return true; } yy += h + 8; } }
    return false;
}
int main(void) { OwApp a = { .name = "Mitteilungen", .wm_class = "ow-notifications", .tick_ms = 1000, .draw = draw, .gesture = gesture }; return ow_app_run(&a); }
