#define _POSIX_C_SOURCE 200809L
#include "owapp.h"
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ow_app_run(OwApp *a) {
    if (!ow_win_open(&a->win, a->name, a->wm_class)) return 1;
    Atom ow_back = XInternAtom(a->win.dpy, "OW_BACK", False);
    OwGestureTracker gt; ow_gesture_init(&gt);
    ow_state_load(&a->st, ow_state_path());
    bool dirty = true; long last_tick = ow_now_ms();
    while (!a->quit) {
        XEvent ev;
        while (ow_win_next(&a->win, &ev, 0) || (!dirty && ow_win_next(&a->win, &ev, 50))) {
            OwGesture g = OW_G_NONE;
            switch (ev.type) {
            case Expose: dirty = true; break;
            case ButtonPress: if (ev.xbutton.button == 1) ow_gesture_press(&gt, ev.xbutton.x, ev.xbutton.y, ow_now_ms()); break;
            case MotionNotify: g = ow_gesture_motion(&gt, ev.xmotion.x, ev.xmotion.y, ow_now_ms()); if (g == OW_G_LONGPRESS && a->gesture) dirty |= a->gesture(a, g, gt.x0, gt.y0); break;
            case ButtonRelease: if (ev.xbutton.button == 1) { g = ow_gesture_release(&gt, ev.xbutton.x, ev.xbutton.y, ow_now_ms());
                                    if (g != OW_G_NONE && a->gesture) dirty |= a->gesture(a, g, gt.x0, gt.y0); } break;
            case KeyPress: { KeySym k = XLookupKeysym(&ev.xkey, 0);
                if (k == XK_Escape || k == XK_BackSpace) a->quit = true; else if (a->key) dirty |= a->key(a, k); break; }
            case ClientMessage:
                if ((Atom)ev.xclient.data.l[0] == a->win.wm_delete || ev.xclient.message_type == ow_back) a->quit = true;
                break;
            }
            if (a->quit) break;
        }
        long now = ow_now_ms();
        if (ow_gesture_poll(&gt, now) == OW_G_LONGPRESS && a->gesture) dirty |= a->gesture(a, OW_G_LONGPRESS, gt.x0, gt.y0);
        if (a->tick_ms > 0 && now - last_tick >= a->tick_ms) { last_tick = now; ow_state_load(&a->st, ow_state_path()); dirty = true; }
        if (dirty && !a->quit) {
            ow_clear(a->win.cr); a->draw(a, a->win.cr); ow_win_flush(&a->win); dirty = false;
        }
    }
    ow_win_close(&a->win); return 0;
}
