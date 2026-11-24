/* App-Starter: 2 Seiten, Wisch links/rechts blättert, Tipp startet die App (als eigener Prozess) */
#define _POSIX_C_SOURCE 200809L
#include "../widgets/owapp.h"
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
typedef struct { const char *label, *icon, *bin; OwColor c; } AppEntry;
static const AppEntry apps[] = {
    {"Einstellungen", "gear", "ow-settings", {0.45, 0.50, 0.62}}, {"Akku", "battery", "ow-battery", {0.30, 0.85, 0.52}},
    {"System", "info", "ow-sysinfo", {0.25, 0.62, 1.0}}, {"Mitteilungen", "bell", "ow-notifications", {1.0, 0.72, 0.20}},
};
#define N ((int)(sizeof apps / sizeof apps[0]))
static char bindir[512];
static const double px[] = {112, 248, 112, 248}, py[] = {130, 130, 250, 250};   /* 2x2, jedes Ziel > 80 px, Platz für Beschriftung */
static void draw(OwApp *a, cairo_t *cr) {
    (void)a; ow_text(cr, "Apps", 180, 50, 16, OW_DIM, 0, false);
    for (int i = 0; i < N; i++) {
        cairo_pattern_t *g = cairo_pattern_create_radial(px[i] - 10, py[i] - 12, 4, px[i], py[i], 42);
        cairo_pattern_add_color_stop_rgba(g, 0, apps[i].c.r, apps[i].c.g, apps[i].c.b, 0.42); cairo_pattern_add_color_stop_rgba(g, 1, apps[i].c.r, apps[i].c.g, apps[i].c.b, 0.14);
        cairo_new_path(cr); cairo_arc(cr, px[i], py[i], 41, 0, 2 * M_PI); cairo_set_source(cr, g); cairo_fill(cr); cairo_pattern_destroy(g);
        ow_icon(cr, apps[i].icon, px[i], py[i] - 1, 36, apps[i].c);
        ow_text(cr, apps[i].label, px[i], py[i] + 56, 11, OW_DIM, 0, false);
    }
}
static bool gesture(OwApp *a, OwGesture g, int x, int y) {
    (void)a; if (g != OW_G_TAP) return false;
    for (int i = 0; i < N; i++) if (hypot(x - px[i], y - py[i]) < 46) {
        if (fork() == 0) { char p[600]; snprintf(p, sizeof p, "%s/%s", bindir, apps[i].bin); execl(p, p, (char *)NULL); execlp(apps[i].bin, apps[i].bin, (char *)NULL); _exit(127); }
        return false; }
    return false;
}
int main(int argc, char **argv) {
    (void)argc; char self[512]; ssize_t n = readlink("/proc/self/exe", self, sizeof self - 1); self[n > 0 ? n : 0] = 0;
    snprintf(bindir, sizeof bindir, "%s", dirname(self)); signal(SIGCHLD, SIG_IGN); (void)argv;
    OwApp a = { .name = "Apps", .wm_class = "ow-launcher", .tick_ms = 0, .draw = draw, .gesture = gesture }; return ow_app_run(&a);
}
