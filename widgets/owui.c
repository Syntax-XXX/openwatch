/* libowui – Umsetzung. Siehe owui.h */
#define _POSIX_C_SOURCE 200809L
#include "owui.h"
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <cairo/cairo-xlib.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/stat.h>

const OwColor OW_BG = {0, 0, 0}, OW_FG = {0.96, 0.96, 0.97}, OW_DIM = {0.55, 0.57, 0.62},
              OW_ACCENT = {0.25, 0.62, 1.0}, OW_GOOD = {0.30, 0.85, 0.52}, OW_WARN = {1.0, 0.72, 0.20},
              OW_BAD = {1.0, 0.35, 0.35}, OW_CARD = {0.11, 0.115, 0.13};

void ow_set(cairo_t *cr, OwColor c, double a) { cairo_set_source_rgba(cr, c.r, c.g, c.b, a); }

/* ================= Zustand ================= */
void ow_state_default(OwState *s) {
    memset(s, 0, sizeof *s);
    s->battery = 82; s->steps = 4312; s->heart_rate = 72; s->temp_c = 31.5; s->brightness = 60;
    s->wifi = true; s->bluetooth = true;
}
const char *ow_state_path(void) { const char *p = getenv("OW_STATE"); return p ? p : "/tmp/openwatch/state.conf"; }
const char *ow_settings_path(void) { const char *p = getenv("OW_SETTINGS"); return p ? p : "/tmp/openwatch/settings.conf"; }

static void apply_kv(OwState *s, const char *k, const char *v) {
    if (!strcmp(k, "battery")) s->battery = atoi(v);
    else if (!strcmp(k, "charging")) s->charging = atoi(v);
    else if (!strcmp(k, "steps")) s->steps = atoi(v);
    else if (!strcmp(k, "heart_rate")) s->heart_rate = atoi(v);
    else if (!strcmp(k, "temp_c")) s->temp_c = atof(v);
    else if (!strcmp(k, "brightness")) s->brightness = atoi(v);
    else if (!strcmp(k, "wifi")) s->wifi = atoi(v);
    else if (!strcmp(k, "bluetooth")) s->bluetooth = atoi(v);
    else if (!strcmp(k, "dnd")) s->dnd = atoi(v);
    else if (!strcmp(k, "always_on")) s->always_on = atoi(v);
    else if (!strcmp(k, "airplane")) s->airplane = atoi(v);
    else if (!strncmp(k, "notif", 5) && s->notif_count < OW_MAX_NOTIF) {
        /* notif=App|Titel|Text|Zeit */
        OwNotif *n = &s->notif[s->notif_count];
        char buf[256]; snprintf(buf, sizeof buf, "%s", v);
        char *f[4] = {0}; int i = 0; for (char *t = strtok(buf, "|"); t && i < 4; t = strtok(NULL, "|")) f[i++] = t;
        if (i == 4) { snprintf(n->app, sizeof n->app, "%s", f[0]); snprintf(n->title, sizeof n->title, "%s", f[1]);
                      snprintf(n->text, sizeof n->text, "%s", f[2]); snprintf(n->time, sizeof n->time, "%s", f[3]); s->notif_count++; }
    }
}
static bool load_file(OwState *s, const char *path) {
    FILE *f = fopen(path, "r"); if (!f) return false;
    char line[320];
    while (fgets(line, sizeof line, f)) {
        line[strcspn(line, "\r\n")] = 0;
        char *eq = strchr(line, '='); if (!eq || line[0] == '#') continue;
        *eq = 0; apply_kv(s, line, eq + 1);
    }
    fclose(f); return true;
}
bool ow_state_load(OwState *s, const char *path) {
    /* Sensorwerte/Benachrichtigungen aus state.conf, Schalter (vom Nutzer gesetzt) aus settings.conf – letztere gewinnen */
    OwState tmp; ow_state_default(&tmp);
    bool ok = load_file(&tmp, path);
    load_file(&tmp, ow_settings_path());
    *s = tmp; return ok;
}
bool ow_state_save_toggles(const OwState *s, const char *path) {
    (void)path;
    FILE *f = fopen(ow_settings_path(), "w"); if (!f) return false;
    fprintf(f, "# von OpenWatch Settings gesetzt\nwifi=%d\nbluetooth=%d\ndnd=%d\nalways_on=%d\nairplane=%d\nbrightness=%d\n",
            s->wifi, s->bluetooth, s->dnd, s->always_on, s->airplane, s->brightness);
    fclose(f); return true;
}

/* ================= Gesten ================= */
long ow_now_ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1000L + t.tv_nsec / 1000000L; }
void ow_gesture_init(OwGestureTracker *g) { memset(g, 0, sizeof *g); }
OwGesture ow_gesture_press(OwGestureTracker *g, int x, int y, long now) {
    g->down = true; g->x0 = g->x = x; g->y0 = g->y = y; g->t0_ms = now; g->long_fired = false; return OW_G_NONE;
}
OwGesture ow_gesture_poll(OwGestureTracker *g, long now) {
    if (!g->down || g->long_fired) return OW_G_NONE;
    int dx = g->x - g->x0, dy = g->y - g->y0;
    if (now - g->t0_ms >= 600 && dx * dx + dy * dy < 12 * 12) { g->long_fired = true; return OW_G_LONGPRESS; }
    return OW_G_NONE;
}
OwGesture ow_gesture_motion(OwGestureTracker *g, int x, int y, long now) {
    if (!g->down) return OW_G_NONE;
    g->x = x; g->y = y;
    return ow_gesture_poll(g, now);
}
OwGesture ow_gesture_release(OwGestureTracker *g, int x, int y, long now) {
    if (!g->down) return OW_G_NONE;
    g->down = false; (void)now;
    int dx = x - g->x0, dy = y - g->y0, adx = abs(dx), ady = abs(dy);
    if (g->long_fired) return OW_G_NONE;
    if (adx < 14 && ady < 14) return OW_G_TAP;
    if (adx >= 40 || ady >= 40) {
        if (adx > ady) return dx < 0 ? OW_G_SWIPE_LEFT : OW_G_SWIPE_RIGHT;
        return dy < 0 ? OW_G_SWIPE_UP : OW_G_SWIPE_DOWN;
    }
    return OW_G_NONE;
}

/* ================= Fenster ================= */
bool ow_win_open(OwWin *w, const char *title, const char *wm_class) {
    memset(w, 0, sizeof *w);
    w->dpy = XOpenDisplay(NULL);
    if (!w->dpy) { fprintf(stderr, "%s: kein X-Display (DISPLAY gesetzt?)\n", title); return false; }
    w->screen = DefaultScreen(w->dpy);
    XSetWindowAttributes at = {0};
    at.background_pixel = BlackPixel(w->dpy, w->screen);
    at.event_mask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | KeyPressMask | StructureNotifyMask;
    w->win = XCreateWindow(w->dpy, RootWindow(w->dpy, w->screen), 0, 0, OW_W, OW_H, 0, CopyFromParent, InputOutput,
                           CopyFromParent, CWBackPixel | CWEventMask, &at);
    XStoreName(w->dpy, w->win, title);
    XClassHint ch = { (char *)wm_class, (char *)"OpenWatch" }; XSetClassHint(w->dpy, w->win, &ch);
    XSizeHints sh = { .flags = PMinSize | PMaxSize | PPosition, .min_width = OW_W, .min_height = OW_H, .max_width = OW_W, .max_height = OW_H };
    XSetWMNormalHints(w->dpy, w->win, &sh);
    w->wm_delete = XInternAtom(w->dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(w->dpy, w->win, &w->wm_delete, 1);
    XMapWindow(w->dpy, w->win);
    w->surf = cairo_xlib_surface_create(w->dpy, w->win, DefaultVisual(w->dpy, w->screen), OW_W, OW_H);
    w->cr = cairo_create(w->surf);
    return true;
}
void ow_win_flush(OwWin *w) { cairo_surface_flush(w->surf); XFlush(w->dpy); }
void ow_win_close(OwWin *w) {
    if (w->cr) cairo_destroy(w->cr);
    if (w->surf) cairo_surface_destroy(w->surf);
    if (w->dpy) { XDestroyWindow(w->dpy, w->win); XCloseDisplay(w->dpy); }
}
bool ow_win_next(OwWin *w, XEvent *ev, int timeout_ms) {
    if (XPending(w->dpy)) { XNextEvent(w->dpy, ev); return true; }
    if (timeout_ms <= 0) return false;
    int fd = ConnectionNumber(w->dpy);
    fd_set fds; FD_ZERO(&fds); FD_SET(fd, &fds);
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    if (select(fd + 1, &fds, NULL, NULL, &tv) > 0 && XPending(w->dpy)) { XNextEvent(w->dpy, ev); return true; }
    return false;
}

/* ================= Zeichnen ================= */
void ow_clear(cairo_t *cr) { cairo_set_source_rgb(cr, 0, 0, 0); cairo_paint(cr); }

static void font(cairo_t *cr, double size, bool bold) {
    cairo_select_font_face(cr, "Inter", CAIRO_FONT_SLANT_NORMAL, bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);   /* fehlt Inter, nimmt Cairo die Systemschrift */
    cairo_set_font_size(cr, size);
}
void ow_text(cairo_t *cr, const char *s, double x, double y, double size, OwColor c, int align, bool bold) {
    font(cr, size, bold); cairo_text_extents_t e; cairo_text_extents(cr, s, &e);
    double px = align == 0 ? x - (e.width / 2 + e.x_bearing) : align > 0 ? x - e.width - e.x_bearing : x - e.x_bearing;
    ow_set(cr, c, 1); cairo_move_to(cr, px, y); cairo_show_text(cr, s);
    cairo_new_path(cr);   /* sonst zieht der nächste Bogen eine Linie vom Textende */
}
void ow_text_fit(cairo_t *cr, const char *s, double x, double y, double maxw, double size, OwColor c, int align, bool bold) {
    font(cr, size, bold); cairo_text_extents_t e; cairo_text_extents(cr, s, &e);
    if (e.width <= maxw) { ow_text(cr, s, x, y, size, c, align, bold); return; }
    char buf[160]; snprintf(buf, sizeof buf, "%s", s);
    for (int n = (int)strlen(buf); n > 1; n--) {
        buf[n] = 0; if (n > 3) { buf[n - 1] = '.'; buf[n - 2] = '.'; }
        cairo_text_extents(cr, buf, &e); if (e.width <= maxw) break;
    }
    ow_text(cr, buf, x, y, size, c, align, bold);
}
void ow_round_rect(cairo_t *cr, double x, double y, double w, double h, double r) {
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r, r, -M_PI / 2, 0); cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2, M_PI); cairo_arc(cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_close_path(cr);
}
void ow_pill(cairo_t *cr, double x, double y, double w, double h, OwColor bg, double alpha) {
    ow_round_rect(cr, x, y, w, h, h / 2); ow_set(cr, bg, alpha); cairo_fill(cr);
}
void ow_ring(cairo_t *cr, double cx, double cy, double rad, double width, double frac, OwColor c) {
    cairo_set_line_width(cr, width); cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    ow_set(cr, OW_CARD, 1); cairo_arc(cr, cx, cy, rad, 0, 2 * M_PI); cairo_stroke(cr);
    if (frac > 0) { ow_set(cr, c, 1); cairo_arc(cr, cx, cy, rad, -M_PI / 2, -M_PI / 2 + 2 * M_PI * (frac > 1 ? 1 : frac)); cairo_stroke(cr); }
}
void ow_circle_mask(cairo_t *cr) {
    cairo_save(cr); cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_rectangle(cr, 0, 0, OW_W, OW_H); cairo_arc(cr, OW_W / 2.0, OW_H / 2.0, OW_W / 2.0, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0, 0, 0); cairo_fill(cr); cairo_restore(cr);
}
void ow_battery_icon(cairo_t *cr, double x, double y, int pct, bool charging) {
    OwColor c = pct <= 15 ? OW_BAD : pct <= 30 ? OW_WARN : OW_GOOD;
    ow_round_rect(cr, x, y, 30, 15, 4); cairo_set_line_width(cr, 1.8); ow_set(cr, OW_DIM, 1); cairo_stroke(cr);
    cairo_rectangle(cr, x + 31.5, y + 4.5, 2.5, 6); ow_set(cr, OW_DIM, 1); cairo_fill(cr);
    ow_round_rect(cr, x + 3, y + 3, 24.0 * pct / 100.0 > 2 ? 24.0 * pct / 100.0 : 2, 9, 2); ow_set(cr, c, 1); cairo_fill(cr);
    if (charging) { ow_text(cr, "⚡", x + 15, y + 12.5, 11, OW_BG, 0, true); }
}
void ow_icon(cairo_t *cr, const char *name, double cx, double cy, double sz, OwColor c) {
    cairo_new_path(cr); cairo_save(cr); cairo_translate(cr, cx, cy); double s = sz / 24.0; cairo_scale(cr, s, s);
    cairo_set_line_width(cr, 2.2); cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    ow_set(cr, c, 1);
    if (!strcmp(name, "wifi")) {
        for (int i = 0; i < 3; i++) { cairo_new_path(cr); cairo_arc(cr, 0, 7, 6 + i * 5, -M_PI * 0.80, -M_PI * 0.20); cairo_stroke(cr); }
        cairo_arc(cr, 0, 8, 1.6, 0, 2 * M_PI); cairo_fill(cr);
    } else if (!strcmp(name, "bluetooth")) {
        cairo_move_to(cr, -6, -5); cairo_line_to(cr, 6, 5); cairo_line_to(cr, 0, 11); cairo_line_to(cr, 0, -11); cairo_line_to(cr, 6, -5); cairo_line_to(cr, -6, 5); cairo_stroke(cr);
    } else if (!strcmp(name, "dnd")) {
        cairo_arc(cr, 0, 0, 10, 0, 2 * M_PI); cairo_stroke(cr); cairo_move_to(cr, -5, 0); cairo_line_to(cr, 5, 0); cairo_stroke(cr);
    } else if (!strcmp(name, "sun")) {
        cairo_arc(cr, 0, 0, 5, 0, 2 * M_PI); cairo_stroke(cr);
        for (int i = 0; i < 8; i++) { double a = i * M_PI / 4; cairo_move_to(cr, cos(a) * 8, sin(a) * 8); cairo_line_to(cr, cos(a) * 11, sin(a) * 11); cairo_stroke(cr); }
    } else if (!strcmp(name, "moon")) {
        cairo_arc(cr, 0, 0, 9, 0.6, 2 * M_PI - 0.6); cairo_arc_negative(cr, 5, -3, 7.5, 2 * M_PI - 1.0, 1.0); cairo_stroke(cr);
    } else if (!strcmp(name, "plane")) {
        cairo_move_to(cr, 0, -11); cairo_line_to(cr, 3, -2); cairo_line_to(cr, 11, 3); cairo_line_to(cr, 11, 6); cairo_line_to(cr, 2, 4); cairo_line_to(cr, 1, 10); cairo_line_to(cr, 4, 12); cairo_line_to(cr, -4, 12); cairo_line_to(cr, -1, 10); cairo_line_to(cr, -2, 4); cairo_line_to(cr, -11, 6); cairo_line_to(cr, -11, 3); cairo_line_to(cr, -3, -2); cairo_close_path(cr); cairo_fill(cr);
    } else if (!strcmp(name, "gear")) {
        cairo_arc(cr, 0, 0, 4, 0, 2 * M_PI); cairo_stroke(cr);
        for (int i = 0; i < 8; i++) { double a = i * M_PI / 4; cairo_move_to(cr, cos(a) * 7, sin(a) * 7); cairo_line_to(cr, cos(a) * 11, sin(a) * 11); cairo_stroke(cr); }
        cairo_arc(cr, 0, 0, 8, 0, 2 * M_PI); cairo_stroke(cr);
    } else if (!strcmp(name, "bell")) {
        cairo_move_to(cr, -8, 6); cairo_curve_to(cr, -8, -2, -6, -9, 0, -9); cairo_curve_to(cr, 6, -9, 8, -2, 8, 6); cairo_close_path(cr); cairo_stroke(cr);
        cairo_arc(cr, 0, 9, 2.2, 0, M_PI); cairo_fill(cr);
    } else if (!strcmp(name, "battery")) {
        ow_round_rect(cr, -10, -6, 18, 12, 3); cairo_stroke(cr); cairo_rectangle(cr, 9, -2.5, 2.5, 5); cairo_fill(cr);
        cairo_move_to(cr, 1, -4); cairo_line_to(cr, -3, 1); cairo_line_to(cr, 1, 1); cairo_line_to(cr, -1, 5); cairo_stroke(cr);
    } else if (!strcmp(name, "info")) {
        cairo_arc(cr, 0, 0, 10, 0, 2 * M_PI); cairo_stroke(cr); cairo_move_to(cr, 0, -1); cairo_line_to(cr, 0, 6); cairo_stroke(cr);
        cairo_arc(cr, 0, -5, 1.4, 0, 2 * M_PI); cairo_fill(cr);
    } else if (!strcmp(name, "grid")) {
        for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2) { ow_round_rect(cr, i * 6 - 4, j * 6 - 4, 8, 8, 2); cairo_fill(cr); }
    } else if (!strcmp(name, "heart")) {
        cairo_move_to(cr, 0, 9); cairo_curve_to(cr, -14, -1, -8, -11, 0, -4); cairo_curve_to(cr, 8, -11, 14, -1, 0, 9); cairo_fill(cr);
    } else if (!strcmp(name, "steps")) {
        cairo_move_to(cr, -8, 8); cairo_line_to(cr, -2, -8); cairo_line_to(cr, 4, 8); cairo_line_to(cr, 10, -8); cairo_stroke(cr);
    } else if (!strcmp(name, "face")) {
        cairo_arc(cr, 0, 0, 10, 0, 2 * M_PI); cairo_stroke(cr); cairo_move_to(cr, 0, 0); cairo_line_to(cr, 0, -6); cairo_move_to(cr, 0, 0); cairo_line_to(cr, 5, 3); cairo_stroke(cr);
    } else if (!strcmp(name, "back")) {
        cairo_move_to(cr, 4, -9); cairo_line_to(cr, -5, 0); cairo_line_to(cr, 4, 9); cairo_stroke(cr);
    }
    cairo_restore(cr);
}
void ow_status_dots(cairo_t *cr, int count, int active, double cy) {
    double w = (count - 1) * 14.0, x0 = OW_W / 2.0 - w / 2;
    for (int i = 0; i < count; i++) { ow_set(cr, i == active ? OW_FG : OW_DIM, i == active ? 1 : 0.55); cairo_arc(cr, x0 + i * 14.0, cy, i == active ? 3.2 : 2.4, 0, 2 * M_PI); cairo_fill(cr); }
}
void ow_title(cairo_t *cr, const char *s) { ow_text(cr, s, OW_W / 2.0, 52, 20, OW_FG, 0, true); }
double ow_ease(double t) { t = t < 0 ? 0 : t > 1 ? 1 : t; return t < 0.5 ? 4 * t * t * t : 1 - pow(-2 * t + 2, 3) / 2; }

/* Dicker Bogen mit Farbverlauf (Pixel-/One-UI-Stil): von-bis in Grad (0 = oben, im Uhrzeigersinn), frac 0..1 des Bogens */
void ow_arc_gradient(cairo_t *cr, double cx, double cy, double rad, double width, double from_deg, double to_deg, double frac, OwColor c1, OwColor c2) {
    double a0 = (from_deg - 90) * M_PI / 180, a1 = (to_deg - 90) * M_PI / 180;
    cairo_save(cr); cairo_new_path(cr); cairo_set_line_width(cr, width); cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    ow_set(cr, OW_CARD, 1); cairo_arc(cr, cx, cy, rad, a0, a1); cairo_stroke(cr);
    if (frac > 0.005) {
        double a = a0 + (a1 - a0) * (frac > 1 ? 1 : frac);
        cairo_pattern_t *g = cairo_pattern_create_linear(cx - rad, cy, cx + rad, cy);
        cairo_pattern_add_color_stop_rgb(g, 0, c1.r, c1.g, c1.b); cairo_pattern_add_color_stop_rgb(g, 1, c2.r, c2.g, c2.b);
        cairo_set_source(cr, g); cairo_arc(cr, cx, cy, rad, a0, a); cairo_stroke(cr); cairo_pattern_destroy(g);
    }
    cairo_restore(cr);
}
/* Fisheye für Listen (Material 3 Expressive): Zeilen schrumpfen und verblassen zum Rand hin. Liefert Skalierung und Deckkraft */
void ow_fisheye(double y_center, double *scale, double *alpha) {
    double d = fabs(y_center - OW_H / 2.0), t = (d - 40.0) / 130.0; t = t < 0 ? 0 : t > 1 ? 1 : t;
    *scale = 1.0 - 0.28 * t * t; *alpha = 1.0 - 0.85 * t * t;
}
