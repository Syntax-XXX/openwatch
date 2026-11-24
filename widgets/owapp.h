/* Mini-Rahmen für OpenWatch-Apps: Fenster, Gesten, Ticker, Beenden über Zurück (WM_DELETE_WINDOW / OW_BACK). */
#ifndef OWAPP_H
#define OWAPP_H
#include "owui.h"
typedef struct OwApp OwApp;
struct OwApp {
    const char *name, *wm_class;
    int tick_ms;                                   /* Neuzeichnen-Takt (0 = nur bei Eingabe) */
    void (*draw)(OwApp *a, cairo_t *cr);
    /* x,y = Position der Geste (bei Wisch: Startpunkt); Rückgabe true = neu zeichnen */
    bool (*gesture)(OwApp *a, OwGesture g, int x, int y);
    bool (*key)(OwApp *a, KeySym k);
    void *user;
    OwState st;                                    /* wird jeden Tick neu geladen */
    OwWin win;
    bool quit;
    double scroll;                                 /* frei nutzbar (z. B. Listenversatz) */
};
int ow_app_run(OwApp *a);
#endif
