/* OpenWatch Shell – Zifferblatt, Widget-Seiten, Schnelleinstellungen, Mitteilungs-Panel, Anpassen-Ansicht.
 * Wisch runter = Schnelleinstellungen, hoch = Mitteilungen, links/rechts = Widget-Seiten, Tipp = App-Starter,
 * Langdruck = Zifferblatt wählen, Zurück (OW_BACK/Esc) = zurück zur Uhr. Alle Daten über libowui (simuliert oder echt). */
#define _DEFAULT_SOURCE
#include "../widgets/owui.h"
#include <X11/keysym.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <libgen.h>

typedef enum { V_FACE, V_QUICK, V_NOTIF, V_CUSTOM } View;
static const char *face_names[] = { "Digital", "Analog", "Minimal" };
#define N_FACES 3
#define N_PAGES 3          /* 0 Aktivität | 1 Zifferblatt | 2 Heute */

typedef struct {
    OwWin win; OwState st; View view; int page; int face;
    double panel;          /* 0..1 Einblendgrad von Quick/Notif */
    double page_pos;       /* animierte Seite (Fließkomma) */
    double custom_x;       /* Versatz in der Anpassen-Ansicht */
    char bindir[512];
} Shell;
static Shell sh;

/* ---------- Hilfen ---------- */
static const char *wd[] = { "So", "Mo", "Di", "Mi", "Do", "Fr", "Sa" };
static const char *mon[] = { "Jan", "Feb", "Mär", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" };
static void get_time(struct tm *t) { time_t n = time(NULL); localtime_r(&n, t); }
static void face_conf(bool save) {
    const char *p = getenv("OW_FACE") ? getenv("OW_FACE") : "/tmp/openwatch/face.conf";
    if (save) { FILE *f = fopen(p, "w"); if (f) { fprintf(f, "%d\n", sh.face); fclose(f); } }
    else { FILE *f = fopen(p, "r"); int v; if (f) { if (fscanf(f, "%d", &v) == 1 && v >= 0 && v < N_FACES) sh.face = v; fclose(f); } }
}
static void launch(const char *bin) {
    if (fork() == 0) {          /* App liegt neben der Shell (installiert) oder in ../apps (Build-Baum); sonst im PATH */
        char p[640]; const char *dirs[] = { "%s/%s", "%s/../apps/%s" };
        for (int i = 0; i < 2; i++) { snprintf(p, sizeof p, dirs[i], sh.bindir, bin); execl(p, p, (char *)NULL); }
        execlp(bin, bin, (char *)NULL); _exit(127);
    }
}

/* ---------- Zifferblätter ---------- */
static void face_digital(cairo_t *cr, const struct tm *t) {
    char b[32];
    /* Randring = Akku (wie Pixel Watch), links/rechts innen zwei dicke Bogen-Komplikationen: Schritte, Puls (wie One UI / Garmin) */
    OwColor bc = sh.st.battery <= 15 ? OW_BAD : sh.st.battery <= 30 ? OW_WARN : OW_GOOD;
    ow_arc_gradient(cr, 180, 180, 174, 5, 0, 360, sh.st.battery / 100.0, bc, OW_ACCENT);
    ow_arc_gradient(cr, 180, 180, 150, 9, 215, 325, sh.st.steps / 8000.0, OW_GOOD, (OwColor){0.55, 1.0, 0.75});
    ow_arc_gradient(cr, 180, 180, 150, 9, 35, 145, (sh.st.heart_rate - 50) / 100.0, (OwColor){1.0, 0.45, 0.45}, OW_WARN);
    snprintf(b, sizeof b, "%s %d. %s", wd[t->tm_wday], t->tm_mday, mon[t->tm_mon]);
    ow_text(cr, b, 180, 112, 18, OW_DIM, 0, false);
    snprintf(b, sizeof b, "%02d:%02d", t->tm_hour, t->tm_min);
    ow_text(cr, b, 180, 206, 92, OW_FG, 0, true);
    snprintf(b, sizeof b, "%02d", t->tm_sec); ow_text(cr, b, 180, 244, 24, OW_ACCENT, 0, false);
    ow_icon(cr, "steps", 76, 214, 20, OW_GOOD); snprintf(b, sizeof b, "%d", sh.st.steps / 100); ow_text(cr, b, 76, 244, 11, OW_DIM, 0, false);
    ow_icon(cr, "heart", 284, 214, 20, OW_BAD); snprintf(b, sizeof b, "%d", sh.st.heart_rate); ow_text(cr, b, 284, 244, 11, OW_DIM, 0, false);
    snprintf(b, sizeof b, "%d%%", sh.st.battery); ow_text(cr, b, 180, 70, 13, OW_DIM, 0, false);
}
static void hand(cairo_t *cr, double ang, double len, double w, OwColor c) {
    cairo_save(cr); cairo_translate(cr, 180, 180); cairo_rotate(cr, ang);
    cairo_set_line_width(cr, w); cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); ow_set(cr, c, 1);
    cairo_move_to(cr, 0, 14); cairo_line_to(cr, 0, -len); cairo_stroke(cr); cairo_restore(cr);
}
static void face_analog(cairo_t *cr, const struct tm *t) {
    char b[32];
    for (int i = 0; i < 60; i++) {
        double a = i * M_PI / 30, r1 = (i % 5 == 0) ? 150 : 162, r2 = 172;
        cairo_set_line_width(cr, (i % 5 == 0) ? 3.2 : 1.2); ow_set(cr, (i % 5 == 0) ? OW_FG : OW_DIM, 1);
        cairo_move_to(cr, 180 + sin(a) * r1, 180 - cos(a) * r1); cairo_line_to(cr, 180 + sin(a) * r2, 180 - cos(a) * r2); cairo_stroke(cr);
    }
    snprintf(b, sizeof b, "%d", t->tm_mday); ow_text(cr, b, 248, 192, 22, OW_ACCENT, 0, true);
    ow_battery_icon(cr, 150, 118, sh.st.battery, sh.st.charging);
    double s = t->tm_sec, m = t->tm_min + s / 60.0, h = (t->tm_hour % 12) + m / 60.0;
    hand(cr, h * M_PI / 6, 86, 7, OW_FG); hand(cr, m * M_PI / 30, 130, 4.5, OW_FG); hand(cr, s * M_PI / 30, 148, 2, OW_ACCENT);
    cairo_arc(cr, 180, 180, 7, 0, 2 * M_PI); ow_set(cr, OW_ACCENT, 1); cairo_fill(cr);
}
static void face_minimal(cairo_t *cr, const struct tm *t) {
    char b[8]; snprintf(b, sizeof b, "%02d", t->tm_hour); ow_text(cr, b, 180, 168, 120, OW_FG, 0, true);
    snprintf(b, sizeof b, "%02d", t->tm_min); ow_text(cr, b, 180, 276, 120, OW_ACCENT, 0, true);
}
static void draw_face_style(cairo_t *cr, int style, const struct tm *t) {
    if (style == 0) face_digital(cr, t); else if (style == 1) face_analog(cr, t); else face_minimal(cr, t);
}

/* ---------- Widget-Seiten ---------- */
static void page_activity(cairo_t *cr) {
    char b[32]; ow_title(cr, "Aktivität");
    ow_ring(cr, 180, 190, 92, 16, sh.st.steps / 8000.0, OW_GOOD);
    ow_ring(cr, 180, 190, 68, 16, sh.st.heart_rate / 160.0, OW_BAD);
    ow_ring(cr, 180, 190, 44, 16, sh.st.battery / 100.0, OW_ACCENT);
    snprintf(b, sizeof b, "%d Schritte", sh.st.steps); ow_text(cr, b, 180, 308, 17, OW_GOOD, 0, true);
    snprintf(b, sizeof b, "%d bpm", sh.st.heart_rate); ow_text(cr, b, 180, 330, 15, OW_BAD, 0, false);
}
static void page_today(cairo_t *cr) {
    ow_title(cr, "Heute");
    ow_icon(cr, "sun", 180, 126, 46, OW_WARN);
    ow_text(cr, "14°", 180, 196, 62, OW_FG, 0, true);
    ow_text(cr, "Sonnig · simuliert", 180, 224, 14, OW_DIM, 0, false);
    ow_round_rect(cr, 56, 246, 248, 56, 20); ow_set(cr, OW_CARD, 1); cairo_fill(cr);
    ow_text(cr, "15:30", 78, 270, 14, OW_ACCENT, -1, true); ow_text(cr, "Kernel-Build prüfen", 78, 290, 15, OW_FG, -1, false);
}

/* ---------- Panels ---------- */
typedef struct { const char *label, *icon; bool *flag; } Tile;
static void tiles(Tile *t) {
    t[0] = (Tile){ "WLAN", "wifi", &sh.st.wifi }; t[1] = (Tile){ "BT", "bluetooth", &sh.st.bluetooth };
    t[2] = (Tile){ "Ruhe", "moon", &sh.st.dnd }; t[3] = (Tile){ "Flug", "plane", &sh.st.airplane };
    t[4] = (Tile){ "Immer an", "face", &sh.st.always_on }; t[5] = (Tile){ "Hell", "sun", NULL };
}
static const double tx[] = { 98, 180, 262, 98, 180, 262 }, ty[] = { 116, 116, 116, 212, 212, 212 };
static void panel_quick(cairo_t *cr) {
    Tile t[6]; tiles(t); ow_text(cr, "Schnellzugriff", 180, 58, 15, OW_DIM, 0, false);
    for (int i = 0; i < 6; i++) {
        bool on = t[i].flag ? *t[i].flag : true;
        cairo_arc(cr, tx[i], ty[i], 37, 0, 2 * M_PI); ow_set(cr, on ? OW_ACCENT : OW_CARD, on ? 0.95 : 1); cairo_fill(cr);
        ow_icon(cr, t[i].icon, tx[i], ty[i] - 3, 30, on ? OW_BG : OW_DIM);
        char b[16]; if (!t[i].flag) snprintf(b, sizeof b, "%d%%", sh.st.brightness); else snprintf(b, sizeof b, "%s", t[i].label);
        ow_text(cr, b, tx[i], ty[i] + 51, 11, OW_DIM, 0, false);
    }
    char b[32]; snprintf(b, sizeof b, "%d%%", sh.st.battery); ow_battery_icon(cr, 146, 290, sh.st.battery, sh.st.charging); ow_text(cr, b, 190, 302, 15, OW_FG, -1, false);
}
static void panel_notif(cairo_t *cr) {
    ow_text(cr, "Mitteilungen", 180, 76, 17, OW_DIM, 0, false);
    if (!sh.st.notif_count) { ow_icon(cr, "bell", 180, 170, 44, OW_DIM); ow_text(cr, "Alles gelesen", 180, 236, 18, OW_DIM, 0, false); return; }
    for (int i = 0; i < sh.st.notif_count && i < 3; i++) {
        OwNotif *n = &sh.st.notif[i]; double y = 96 + i * 76;
        ow_round_rect(cr, 44, y, 272, 68, 20); ow_set(cr, OW_CARD, 1); cairo_fill(cr);
        ow_text(cr, n->app, 62, y + 22, 12, OW_ACCENT, -1, true); ow_text(cr, n->time, 298, y + 22, 12, OW_DIM, 1, false);
        ow_text_fit(cr, n->title, 62, y + 43, 236, 15, OW_FG, -1, true); ow_text_fit(cr, n->text, 62, y + 60, 236, 12, OW_DIM, -1, false);
    }
    if (sh.st.notif_count > 3) { char b[24]; snprintf(b, sizeof b, "+%d weitere", sh.st.notif_count - 3); ow_text(cr, b, 180, 336, 13, OW_DIM, 0, false); }
}
static void view_custom(cairo_t *cr, const struct tm *t) {
    ow_text(cr, "Zifferblatt", 180, 44, 16, OW_DIM, 0, false);
    for (int i = 0; i < N_FACES; i++) {
        double off = (i - sh.face) * 250 + sh.custom_x;
        if (fabs(off) > 330) continue;
        cairo_save(cr); cairo_translate(cr, 180 + off, 180); double s = i == sh.face ? 0.62 : 0.5; cairo_scale(cr, s, s); cairo_translate(cr, -180, -180);
        cairo_arc(cr, 180, 180, 178, 0, 2 * M_PI); cairo_clip(cr); ow_clear(cr); draw_face_style(cr, i, t);
        cairo_reset_clip(cr); cairo_restore(cr);
        cairo_arc(cr, 180 + off, 180, 178 * (i == sh.face ? 0.62 : 0.5), 0, 2 * M_PI); cairo_set_line_width(cr, i == sh.face ? 3 : 1.5);
        ow_set(cr, i == sh.face ? OW_ACCENT : OW_DIM, 1); cairo_stroke(cr);
    }
    ow_text(cr, face_names[sh.face], 180, 330, 20, OW_FG, 0, true);
    ow_text(cr, "Tippen = wählen", 180, 306, 11, OW_DIM, 0, false);
}

/* ---------- Gesamtbild ---------- */
static void render(cairo_t *cr) {
    struct tm t; get_time(&t);
    ow_clear(cr);
    if (sh.view == V_CUSTOM) { view_custom(cr, &t); ow_circle_mask(cr); return; }
    /* horizontale Seiten */
    for (int p = 0; p < N_PAGES; p++) {
        double off = (p - sh.page_pos) * 360; if (fabs(off) >= 360) continue;
        cairo_save(cr); cairo_translate(cr, off, 0);
        if (p == 1) draw_face_style(cr, sh.face, &t); else if (p == 0) page_activity(cr); else page_today(cr);
        cairo_restore(cr);
    }
    ow_status_dots(cr, N_PAGES, sh.page, 348);
    /* Panels schieben über die Seite */
    if (sh.panel > 0.001) {
        double e = ow_ease(sh.panel);
        double y = sh.view == V_QUICK ? -360 * (1 - e) : 360 * (1 - e);
        cairo_save(cr); cairo_translate(cr, 0, y);
        cairo_rectangle(cr, 0, 0, 360, 360); cairo_set_source_rgba(cr, 0.03, 0.035, 0.05, 1.0); cairo_fill(cr);
        if (sh.view == V_QUICK) panel_quick(cr); else panel_notif(cr);
        cairo_restore(cr);
    }
    ow_circle_mask(cr);
}

/* ---------- Eingabe ---------- */
static void back(void) {
    if (sh.view != V_FACE) { sh.view = V_FACE; }
    else if (sh.page != 1) sh.page = 1;
}
static bool handle(OwGesture g, int x, int y) {
    switch (sh.view) {
    case V_FACE:
        if (g == OW_G_SWIPE_DOWN) { sh.view = V_QUICK; sh.panel = 0; }
        else if (g == OW_G_SWIPE_UP) { sh.view = V_NOTIF; sh.panel = 0; }
        else if (g == OW_G_SWIPE_LEFT && sh.page < N_PAGES - 1) sh.page++;
        else if (g == OW_G_SWIPE_RIGHT && sh.page > 0) sh.page--;
        else if (g == OW_G_TAP && sh.page == 1) launch("ow-launcher");
        else if (g == OW_G_LONGPRESS && sh.page == 1) { sh.view = V_CUSTOM; sh.custom_x = 0; }
        else { return false; }
        return true;
    case V_QUICK:
        if (g == OW_G_SWIPE_UP) { sh.view = V_FACE; return true; }
        if (g == OW_G_TAP) { Tile t[6]; tiles(t);
            for (int i = 0; i < 6; i++) if (hypot(x - tx[i], y - ty[i]) < 42) {
                if (t[i].flag) *t[i].flag = !*t[i].flag; else sh.st.brightness = sh.st.brightness >= 100 ? 20 : sh.st.brightness + 20;
                ow_state_save_toggles(&sh.st, NULL); return true; } }
        return false;
    case V_NOTIF:
        if (g == OW_G_SWIPE_DOWN) { sh.view = V_FACE; return true; }
        if (g == OW_G_TAP) { launch("ow-notifications"); return false; }
        return false;
    case V_CUSTOM:
        if (g == OW_G_SWIPE_LEFT && sh.face < N_FACES - 1) sh.face++;
        else if (g == OW_G_SWIPE_RIGHT && sh.face > 0) sh.face--;
        else if (g == OW_G_TAP) { face_conf(true); sh.view = V_FACE; }
        else { return false; }
        return true;
    }
    return false;
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    char self[512]; ssize_t n = readlink("/proc/self/exe", self, sizeof self - 1); self[n > 0 ? n : 0] = 0;
    snprintf(sh.bindir, sizeof sh.bindir, "%s", dirname(self)); signal(SIGCHLD, SIG_IGN);
    sh.view = V_FACE; sh.page = 1; sh.page_pos = 1; sh.face = 0; face_conf(false);
    if (!ow_win_open(&sh.win, "OpenWatch", "ow-shell")) return 1;
    Atom ow_back = XInternAtom(sh.win.dpy, "OW_BACK", False);
    ow_state_load(&sh.st, ow_state_path());
    OwGestureTracker gt; ow_gesture_init(&gt);
    long last_s = 0, last_frame = 0; bool run = true;
    while (run) {
        XEvent ev; bool dirty = false;
        while (ow_win_next(&sh.win, &ev, 0)) {
            OwGesture g = OW_G_NONE;
            switch (ev.type) {
            case Expose: dirty = true; break;
            case ButtonPress: if (ev.xbutton.button == 1) ow_gesture_press(&gt, ev.xbutton.x, ev.xbutton.y, ow_now_ms()); break;
            case MotionNotify:
                g = ow_gesture_motion(&gt, ev.xmotion.x, ev.xmotion.y, ow_now_ms());
                if (g == OW_G_LONGPRESS) dirty |= handle(g, gt.x0, gt.y0);
                if (sh.view == V_CUSTOM && gt.down) { sh.custom_x = (ev.xmotion.x - gt.x0) * 0.6; dirty = true; }
                break;
            case ButtonRelease: if (ev.xbutton.button == 1) { sh.custom_x = 0; g = ow_gesture_release(&gt, ev.xbutton.x, ev.xbutton.y, ow_now_ms()); if (g) dirty |= handle(g, gt.x0, gt.y0); dirty = true; } break;
            case KeyPress: { KeySym k = XLookupKeysym(&ev.xkey, 0);
                if (k == XK_Escape) { back(); dirty = true; }
                else if (k == XK_Left) dirty |= handle(OW_G_SWIPE_RIGHT, 0, 0); else if (k == XK_Right) dirty |= handle(OW_G_SWIPE_LEFT, 0, 0);
                else if (k == XK_Up) dirty |= handle(OW_G_SWIPE_UP, 0, 0); else if (k == XK_Down) dirty |= handle(OW_G_SWIPE_DOWN, 0, 0);
                else if (k == XK_Return) dirty |= handle(OW_G_TAP, 0, 0); break; }
            case ClientMessage:
                if (ev.xclient.message_type == ow_back) { back(); dirty = true; } break;
            }
        }
        long now = ow_now_ms();
        if (ow_gesture_poll(&gt, now) == OW_G_LONGPRESS) dirty |= handle(OW_G_LONGPRESS, gt.x0, gt.y0);
        if (now / 1000 != last_s) { last_s = now / 1000; ow_state_load(&sh.st, ow_state_path()); dirty = true; }
        /* Animationen */
        double tp = (sh.view == V_QUICK || sh.view == V_NOTIF) ? 1.0 : 0.0;
        if (fabs(sh.panel - tp) > 0.002) { sh.panel += (tp - sh.panel) * 0.35; if (fabs(sh.panel - tp) < 0.01) sh.panel = tp; dirty = true; }
        if (fabs(sh.page_pos - sh.page) > 0.002) { sh.page_pos += (sh.page - sh.page_pos) * 0.35; if (fabs(sh.page_pos - sh.page) < 0.01) sh.page_pos = sh.page; dirty = true; }
        bool animating = fabs(sh.panel - tp) > 0.002 || fabs(sh.page_pos - sh.page) > 0.002;
        if (dirty && now - last_frame >= (animating ? 16 : 0)) { last_frame = now; render(sh.win.cr); ow_win_flush(&sh.win); }
        else if (!dirty) { XEvent peek; if (!XPending(sh.win.dpy)) { (void)peek; usleep(animating ? 8000 : 30000); } }
    }
    ow_win_close(&sh.win); return 0;
}
