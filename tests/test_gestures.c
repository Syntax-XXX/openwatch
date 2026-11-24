#include <assert.h>
#include <stdio.h>
#include "owui.h"
static OwGesture run(int x0, int y0, int x1, int y1, long dt) {
    OwGestureTracker g; ow_gesture_init(&g); ow_gesture_press(&g, x0, y0, 0); return ow_gesture_release(&g, x1, y1, dt);
}
int main(void) {
    assert(run(180, 180, 182, 181, 80) == OW_G_TAP);
    assert(run(300, 180, 100, 190, 200) == OW_G_SWIPE_LEFT);
    assert(run(60, 180, 260, 170, 200) == OW_G_SWIPE_RIGHT);
    assert(run(180, 300, 175, 100, 200) == OW_G_SWIPE_UP);
    assert(run(180, 60, 185, 260, 200) == OW_G_SWIPE_DOWN);
    assert(run(180, 180, 200, 190, 100) == OW_G_NONE);          /* zu kurz für Wisch, zu weit für Tipp */
    OwGestureTracker g; ow_gesture_init(&g); ow_gesture_press(&g, 180, 180, 0);
    assert(ow_gesture_motion(&g, 181, 180, 300) == OW_G_NONE);
    assert(ow_gesture_motion(&g, 181, 181, 650) == OW_G_LONGPRESS);   /* Langdruck nach 600 ms */
    assert(ow_gesture_release(&g, 181, 181, 700) == OW_G_NONE);       /* danach kein zusätzlicher Tipp */
    ow_gesture_init(&g); ow_gesture_press(&g, 50, 50, 0);
    assert(ow_gesture_poll(&g, 300) == OW_G_NONE && ow_gesture_poll(&g, 700) == OW_G_LONGPRESS && ow_gesture_poll(&g, 900) == OW_G_NONE);   /* ohne Bewegung, nur einmal */
    puts("gestures ok"); return 0;
}
