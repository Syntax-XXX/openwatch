#include <assert.h>
#include <stdio.h>
#include "owm_stack.h"
int main(void) {
    OwmStack s; owm_stack_init(&s);
    assert(owm_stack_top(&s) == 0 && owm_stack_back_target(&s) == 0);
    assert(owm_stack_push(&s, 10, true));               /* Shell = Basis */
    assert(!owm_stack_push(&s, 10, true));              /* doppelt abgelehnt */
    assert(owm_stack_back_target(&s) == 0);             /* Basis wird nie geschlossen */
    assert(owm_stack_push(&s, 20, false) && owm_stack_push(&s, 30, false));
    assert(owm_stack_top(&s) == 30 && owm_stack_back_target(&s) == 30);
    assert(owm_stack_remove(&s, 30));                   /* oberstes entfernt */
    assert(owm_stack_top(&s) == 20);
    assert(!owm_stack_remove(&s, 10));                  /* mittleres/unteres: nicht oben */
    assert(owm_stack_top(&s) == 20);
    assert(owm_stack_remove(&s, 20) && owm_stack_top(&s) == 0);
    for (unsigned long i = 1; i <= OWM_MAX; i++) assert(owm_stack_push(&s, i, false));
    assert(!owm_stack_push(&s, 999, false));            /* voll */
    puts("stack ok"); return 0;
}
