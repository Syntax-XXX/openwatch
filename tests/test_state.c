#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "owui.h"
int main(void) {
    const char *p = "/tmp/ow-test-state.conf", *q = "/tmp/ow-test-settings.conf";
    setenv("OW_SETTINGS", q, 1); remove(q);
    FILE *f = fopen(p, "w"); fputs("battery=17\ncharging=1\nsteps=999\nnotif0=App|Titel|Text mit Inhalt|12:00\nnotif1=A2|T2|X2|12:01\n", f); fclose(f);
    OwState s; assert(ow_state_load(&s, p));
    assert(s.battery == 17 && s.charging && s.steps == 999 && s.notif_count == 2);
    assert(!strcmp(s.notif[0].title, "Titel") && !strcmp(s.notif[1].time, "12:01"));
    s.wifi = false; s.dnd = true; s.brightness = 40; assert(ow_state_save_toggles(&s, NULL));
    OwState t; ow_state_load(&t, p); assert(!t.wifi && t.dnd && t.brightness == 40 && t.battery == 17);   /* Schalter überlagern die Sensorwerte */
    assert(!ow_state_load(&t, "/tmp/gibt-es-nicht.conf") && t.battery == 82);                            /* fehlende Datei -> Standardwerte */
    remove(p); remove(q); puts("state ok"); return 0;
}
