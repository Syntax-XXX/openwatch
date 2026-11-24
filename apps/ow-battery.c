/* Akku-Info: Ring, Zustand, geschätzte Restzeit (aus simulierten Werten), Temperatur */
#include "../widgets/owapp.h"
#include <stdio.h>
static void draw(OwApp *a, cairo_t *cr) {
    OwState *s = &a->st; char b[64];
    OwColor c = s->battery <= 15 ? OW_BAD : s->battery <= 30 ? OW_WARN : OW_GOOD;
    ow_ring(cr, 180, 168, 104, 14, s->battery / 100.0, c);
    snprintf(b, sizeof b, "%d%%", s->battery); ow_text(cr, b, 180, 186, 64, OW_FG, 0, true);
    ow_text(cr, s->charging ? "lädt" : "entlädt", 180, 214, 18, s->charging ? OW_GOOD : OW_DIM, 0, false);
    ow_icon(cr, "battery", 180, 112, 26, c);
    double h = s->charging ? (100 - s->battery) * 0.022 : s->battery * 0.31;   /* grobe Simulation */
    snprintf(b, sizeof b, s->charging ? "voll in ca. %dh %02dmin" : "noch ca. %dh %02dmin", (int)h, (int)((h - (int)h) * 60));
    ow_text(cr, b, 180, 300, 17, OW_FG, 0, false);
    snprintf(b, sizeof b, "%.1f °C", s->temp_c); ow_text(cr, b, 180, 326, 15, OW_DIM, 0, false);
}
int main(void) { OwApp a = { .name = "Akku", .wm_class = "ow-battery", .tick_ms = 1000, .draw = draw }; return ow_app_run(&a); }
