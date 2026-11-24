/* libowui – gemeinsame Zeichen-, Gesten- und Datenhelfer für OpenWatch Shell und Apps.
 * Unabhängig von Gerätetreibern: Daten kommen aus einer Textdatei (OW_STATE), die auf dem PC der
 * Simulator `ow-simd` schreibt und auf der Uhr später ein echter Systemdienst. Nur Xlib + Cairo. */
#ifndef OWUI_H
#define OWUI_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <X11/Xlib.h>
#include <cairo/cairo.h>
#include <stdbool.h>

#define OW_W 360
#define OW_H 360

/* ---------- Farben (AMOLED: echtes Schwarz, wenig Fläche) ---------- */
typedef struct { double r, g, b; } OwColor;
extern const OwColor OW_BG, OW_FG, OW_DIM, OW_ACCENT, OW_GOOD, OW_WARN, OW_BAD, OW_CARD;
void ow_set(cairo_t *cr, OwColor c, double alpha);

/* ---------- Zustand (simuliert oder echt) ---------- */
#define OW_MAX_NOTIF 8
typedef struct { char app[24]; char title[48]; char text[96]; char time[8]; } OwNotif;
typedef struct {
    int battery;          /* 0..100 */
    bool charging;
    int steps, heart_rate;
    double temp_c;        /* Akku-/Gehäusetemperatur */
    int brightness;       /* 0..100 */
    bool wifi, bluetooth, dnd, always_on, airplane;
    int notif_count;
    OwNotif notif[OW_MAX_NOTIF];
} OwState;
void ow_state_default(OwState *s);
bool ow_state_load(OwState *s, const char *path);          /* false = Datei fehlt (Standardwerte bleiben) */
bool ow_state_save_toggles(const OwState *s, const char *path); /* schreibt nur die Schalter zurück (settings.conf) */
const char *ow_state_path(void);
const char *ow_settings_path(void);

/* ---------- Gesten ---------- */
typedef enum { OW_G_NONE, OW_G_TAP, OW_G_LONGPRESS, OW_G_SWIPE_LEFT, OW_G_SWIPE_RIGHT, OW_G_SWIPE_UP, OW_G_SWIPE_DOWN } OwGesture;
typedef struct {
    bool down; int x0, y0, x, y; long t0_ms; bool long_fired;
} OwGestureTracker;
void ow_gesture_init(OwGestureTracker *g);
OwGesture ow_gesture_press(OwGestureTracker *g, int x, int y, long now_ms);
OwGesture ow_gesture_motion(OwGestureTracker *g, int x, int y, long now_ms);   /* Langdruck, solange gedrückt */
OwGesture ow_gesture_poll(OwGestureTracker *g, long now_ms);                   /* Langdruck auch ohne Bewegung: regelmäßig aufrufen */
OwGesture ow_gesture_release(OwGestureTracker *g, int x, int y, long now_ms);  /* Tap oder Wisch */
long ow_now_ms(void);

/* ---------- Fenster ---------- */
typedef struct {
    Display *dpy; Window win; int screen;
    cairo_surface_t *surf; cairo_t *cr;
    Atom wm_delete;
} OwWin;
bool ow_win_open(OwWin *w, const char *title, const char *wm_class);
void ow_win_flush(OwWin *w);
void ow_win_close(OwWin *w);
/* nächstes Ereignis abholen; wartet höchstens timeout_ms (0 = nicht warten). true = Ereignis in *ev */
bool ow_win_next(OwWin *w, XEvent *ev, int timeout_ms);

/* ---------- Zeichnen ---------- */
void ow_clear(cairo_t *cr);                                   /* schwarz */
void ow_text(cairo_t *cr, const char *s, double x, double y, double size, OwColor c, int align /*-1 links,0 Mitte,1 rechts*/, bool bold);
void ow_text_fit(cairo_t *cr, const char *s, double x, double y, double maxw, double size, OwColor c, int align, bool bold);
void ow_round_rect(cairo_t *cr, double x, double y, double w, double h, double r);
void ow_ring(cairo_t *cr, double cx, double cy, double rad, double width, double frac, OwColor c);  /* Fortschrittsring */
void ow_pill(cairo_t *cr, double x, double y, double w, double h, OwColor bg, double alpha);
void ow_circle_mask(cairo_t *cr);                              /* alles außerhalb des 360er-Kreises schwarz */
void ow_battery_icon(cairo_t *cr, double x, double y, int pct, bool charging);
void ow_icon(cairo_t *cr, const char *name, double cx, double cy, double size, OwColor c);  /* einfache Vektor-Symbole */
void ow_status_dots(cairo_t *cr, int count, int active, double cy);   /* Seiten-Punkte */
void ow_title(cairo_t *cr, const char *s);                     /* Seitenkopf oben, rund-sicher */
double ow_ease(double t);
void ow_arc_gradient(cairo_t *cr, double cx, double cy, double rad, double width, double from_deg, double to_deg, double frac, OwColor c1, OwColor c2);
int ow_text_wrap(cairo_t *cr, const char *s, double x, double y, double maxw, double size, double line_h, int max_lines, OwColor c);
void ow_fisheye(double y_center, double *scale, double *alpha);                                      /* easeInOutCubic 0..1 */

#endif
