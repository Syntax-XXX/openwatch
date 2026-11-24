/* owm – OpenWatch Window Manager. Ein Fenster = ein Vollbild (360x360, rund beschnitten), neues Fenster gleitet von
 * rechts herein. Zurück: Wisch vom linken Rand nach rechts, Taste Escape/XF86Back, oder `owm`-Kommando "back".
 * Das Fenster der Klasse "ow-shell" ist die Basis und wird nie geschlossen. Nur Xlib (+ XShape). */
#define _POSIX_C_SOURCE 200809L
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/XF86keysym.h>
#include <X11/extensions/shape.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "owm_stack.h"

#define W 360
#define H 360
#define EDGE 18

static Display *dpy; static Window root, edge;
static OwmStack stack; static bool verbose; static Atom wm_protocols, wm_delete, ow_back_atom;

static void logf_(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#include <stdarg.h>
static void logf_(const char *fmt, ...) { if (!verbose) return; va_list ap; va_start(ap, fmt); fputs("owm: ", stderr); vfprintf(stderr, fmt, ap); fputc('\n', stderr); va_end(ap); }

static int xerr(Display *d, XErrorEvent *e) { (void)d; (void)e; return 0; }   /* Fenster können jederzeit verschwinden */

static void msleep(int ms) { struct timespec t = { 0, ms * 1000000L }; nanosleep(&t, NULL); }

static void make_round(Window w) {
    int ev, er; if (!XShapeQueryExtension(dpy, &ev, &er)) return;
    Pixmap m = XCreatePixmap(dpy, w, W, H, 1);
    GC gc = XCreateGC(dpy, m, 0, NULL);
    XSetForeground(dpy, gc, 0); XFillRectangle(dpy, m, gc, 0, 0, W, H);
    XSetForeground(dpy, gc, 1); XFillArc(dpy, m, gc, 0, 0, W, H, 0, 360 * 64);
    XShapeCombineMask(dpy, w, ShapeBounding, 0, 0, m, ShapeSet);
    XFreeGC(dpy, gc); XFreePixmap(dpy, m);
}
static bool is_base_window(Window w) {
    XClassHint ch; bool base = false;
    if (XGetClassHint(dpy, w, &ch)) { base = ch.res_name && !strcmp(ch.res_name, "ow-shell"); XFree(ch.res_name); XFree(ch.res_class); }
    return base;
}
static void raise_edge(void) { XRaiseWindow(dpy, edge); }
static void focus_top(void) {
    unsigned long t = owm_stack_top(&stack);
    if (t) { XSetInputFocus(dpy, (Window)t, RevertToPointerRoot, CurrentTime); XRaiseWindow(dpy, (Window)t); }
    raise_edge();
}
static void slide_in(Window w) {
    for (int i = 1; i <= 8; i++) { double t = i / 8.0; t = 1 - (1 - t) * (1 - t) * (1 - t); XMoveWindow(dpy, w, (int)(W * (1 - t)), 0); XFlush(dpy); msleep(14); }
    XMoveWindow(dpy, w, 0, 0);
}
static void slide_out(Window w) {
    for (int i = 1; i <= 7; i++) { double t = i / 7.0; t = t * t; XMoveWindow(dpy, w, (int)(W * t), 0); XFlush(dpy); msleep(12); }
}
static void close_window(Window w) {
    slide_out(w);
    Atom *protos; int n; bool polite = false;
    if (XGetWMProtocols(dpy, w, &protos, &n)) { for (int i = 0; i < n; i++) if (protos[i] == wm_delete) polite = true; XFree(protos); }
    if (polite) {
        XEvent e = { .xclient = { .type = ClientMessage, .window = w, .message_type = wm_protocols, .format = 32 } };
        e.xclient.data.l[0] = wm_delete; e.xclient.data.l[1] = CurrentTime; XSendEvent(dpy, w, False, NoEventMask, &e);
    } else XKillClient(dpy, w);
}
static void go_back(void) {
    unsigned long t = owm_stack_back_target(&stack);
    logf_("back -> %s", t ? "schließe oberstes Fenster" : "nichts zu schließen (Basis)");
    if (t) { close_window((Window)t); return; }
    unsigned long base = owm_stack_top(&stack);        /* nur die Basis (Shell) übrig: sie entscheidet selbst (Panel schließen, zur Uhr) */
    if (base) {
        XEvent e = { .xclient = { .type = ClientMessage, .window = (Window)base, .message_type = ow_back_atom, .format = 32 } };
        XSendEvent(dpy, (Window)base, False, NoEventMask, &e);
    }
}
static void on_map_request(XMapRequestEvent *e) {
    Window w = e->window;
    XWindowAttributes a; if (!XGetWindowAttributes(dpy, w, &a) || a.override_redirect) { XMapWindow(dpy, w); return; }
    bool base = is_base_window(w);
    XSetWindowBorderWidth(dpy, w, 0);
    XMoveResizeWindow(dpy, w, base || stack.n == 0 ? 0 : W, 0, W, H);
    make_round(w);
    XSelectInput(dpy, w, StructureNotifyMask | PropertyChangeMask);
    owm_stack_push(&stack, w, base);
    XMapRaised(dpy, w);
    if (!(base || stack.n == 1)) slide_in(w);
    focus_top(); logf_("map 0x%lx base=%d, Stapel=%d", w, base, stack.n);
}
static void on_gone(Window w) {
    if (owm_stack_remove(&stack, w)) { focus_top(); logf_("weg 0x%lx, Stapel=%d", w, stack.n); }
}

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) if (!strcmp(argv[i], "-v")) verbose = true;
    dpy = XOpenDisplay(NULL);
    if (!dpy) { fprintf(stderr, "owm: kein X-Display\n"); return 1; }
    root = DefaultRootWindow(dpy);
    wm_protocols = XInternAtom(dpy, "WM_PROTOCOLS", False); wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    ow_back_atom = XInternAtom(dpy, "OW_BACK", False);
    if (argc > 1 && !strcmp(argv[1], "back")) {   /* Steuer-Kommando: sendet "Zurück" an den laufenden owm */
        XEvent e = { .xclient = { .type = ClientMessage, .window = root, .message_type = ow_back_atom, .format = 32 } };
        XSendEvent(dpy, root, False, SubstructureRedirectMask | SubstructureNotifyMask, &e); XFlush(dpy); return 0;
    }
    XSetErrorHandler(xerr);
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask | PropertyChangeMask);
    XSync(dpy, False);
    owm_stack_init(&stack);
    XSetWindowAttributes at = { .override_redirect = True, .event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask };
    edge = XCreateWindow(dpy, root, 0, 0, EDGE, H, 0, 0, InputOnly, CopyFromParent, CWOverrideRedirect | CWEventMask, &at);
    XMapRaised(dpy, edge);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Escape), AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    XGrabKey(dpy, XKeysymToKeycode(dpy, XF86XK_Back), AnyModifier, root, True, GrabModeAsync, GrabModeAsync);
    XFlush(dpy); logf_("läuft (%dx%d, Rand-Wisch %d px)", W, H, EDGE);
    int ex0 = 0; bool tracking = false;
    for (;;) {
        XEvent e; XNextEvent(dpy, &e);
        switch (e.type) {
        case MapRequest: on_map_request(&e.xmaprequest); break;
        case ConfigureRequest: {           /* Größe/Ort sind fest, nur bestätigen */
            XConfigureRequestEvent *c = &e.xconfigurerequest;
            XWindowChanges ch = { .x = 0, .y = 0, .width = W, .height = H, .border_width = 0, .sibling = c->above, .stack_mode = c->detail };
            XConfigureWindow(dpy, c->window, (unsigned)(c->value_mask & (CWWidth | CWHeight | CWBorderWidth | CWX | CWY)) | CWWidth | CWHeight, &ch);
            break; }
        case UnmapNotify: on_gone(e.xunmap.window); break;
        case DestroyNotify: on_gone(e.xdestroywindow.window); break;
        case ButtonPress: if (e.xbutton.window == edge) { tracking = true; ex0 = e.xbutton.x_root; } break;
        case ButtonRelease: if (e.xbutton.window == edge && tracking) { tracking = false; if (e.xbutton.x_root - ex0 >= 50) go_back(); } break;
        case KeyPress: go_back(); break;
        case ClientMessage: if (e.xclient.message_type == ow_back_atom) go_back(); break;
        }
    }
}
