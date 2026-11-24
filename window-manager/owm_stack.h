/* Fensterstapel des OpenWatch-Fenstermanagers – reine Logik, ohne X11, damit sie einzeln testbar ist. */
#ifndef OWM_STACK_H
#define OWM_STACK_H
#include <stdbool.h>
#define OWM_MAX 16
typedef struct { unsigned long id[OWM_MAX]; bool base[OWM_MAX]; int n; } OwmStack;
void owm_stack_init(OwmStack *s);
bool owm_stack_push(OwmStack *s, unsigned long id, bool is_base);   /* oben drauf; false = voll/doppelt */
bool owm_stack_remove(OwmStack *s, unsigned long id);               /* true = es war das oberste Fenster */
unsigned long owm_stack_top(const OwmStack *s);                     /* 0 = leer */
/* Zurück-Geste: liefert das Fenster, das geschlossen werden soll (nie das Basisfenster), sonst 0 */
unsigned long owm_stack_back_target(const OwmStack *s);
#endif
