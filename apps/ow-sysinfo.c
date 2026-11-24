/* Systeminfo: echte Werte des Rechners/der Uhr (uname, /proc), Display fest 360x360 */
#define _POSIX_C_SOURCE 200809L
#include "../widgets/owapp.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/utsname.h>
static long meminfo(const char *key) { FILE *f = fopen("/proc/meminfo", "r"); char l[128]; long v = -1; if (!f) return -1;
    while (fgets(l, sizeof l, f)) { size_t n = strlen(key); if (!strncmp(l, key, n)) { sscanf(l + n, " %ld", &v); break; } } fclose(f); return v; }
static void row(cairo_t *cr, int i, const char *k, const char *v) {
    double y = 100 + i * 36; ow_text(cr, k, 66, y, 14, OW_DIM, -1, false); ow_text_fit(cr, v, 300, y, 150, 16, OW_FG, 1, true);
}
static void draw(OwApp *a, cairo_t *cr) {
    struct utsname u; uname(&u); char b[160]; (void)a;
    ow_title(cr, "System");
    row(cr, 0, "Gerät", "OpenWatch (Sim)");
    snprintf(b, sizeof b, "%s %s", u.sysname, u.release); row(cr, 1, "Kernel", b);
    snprintf(b, sizeof b, "%s ×%ld", u.machine, sysconf(_SC_NPROCESSORS_ONLN)); row(cr, 2, "CPU", b);
    long t = meminfo("MemTotal:"), f = meminfo("MemAvailable:");
    snprintf(b, sizeof b, "%ld / %ld MB", (t - f) / 1024, t / 1024); row(cr, 3, "RAM", b);
    row(cr, 4, "Display", "360×360 rund");
    FILE *up = fopen("/proc/uptime", "r"); double s = 0; if (up) { fscanf(up, "%lf", &s); fclose(up); }
    snprintf(b, sizeof b, "%dh %02dm", (int)(s / 3600), (int)(s / 60) % 60); row(cr, 5, "Laufzeit", b);
}
int main(void) { OwApp a = { .name = "System", .wm_class = "ow-sysinfo", .tick_ms = 5000, .draw = draw }; return ow_app_run(&a); }
