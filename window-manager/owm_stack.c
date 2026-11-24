#include "owm_stack.h"
#include <string.h>
void owm_stack_init(OwmStack *s) { memset(s, 0, sizeof *s); }
bool owm_stack_push(OwmStack *s, unsigned long id, bool is_base) {
    for (int i = 0; i < s->n; i++) if (s->id[i] == id) return false;
    if (s->n >= OWM_MAX) return false;
    s->id[s->n] = id; s->base[s->n] = is_base; s->n++; return true;
}
bool owm_stack_remove(OwmStack *s, unsigned long id) {
    for (int i = 0; i < s->n; i++) if (s->id[i] == id) {
        bool was_top = (i == s->n - 1);
        memmove(&s->id[i], &s->id[i + 1], (s->n - i - 1) * sizeof s->id[0]);
        memmove(&s->base[i], &s->base[i + 1], (s->n - i - 1) * sizeof s->base[0]);
        s->n--; return was_top;
    }
    return false;
}
unsigned long owm_stack_top(const OwmStack *s) { return s->n ? s->id[s->n - 1] : 0; }
unsigned long owm_stack_back_target(const OwmStack *s) { return (s->n && !s->base[s->n - 1]) ? s->id[s->n - 1] : 0; }
