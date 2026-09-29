#include <stddef.h>

typedef struct {
    int data[16];
    int top;
} Stack;

static inline void stack_init(Stack *s) {
    if (s) s->top = 0;
}

static inline int stack_push(Stack *s, int v) {
    if (!s || s->top >= 16) return -1;
    s->data[s->top++] = v;
    return 0;
}

static inline int stack_pop(Stack *s, int *out) {
    if (!s || s->top == 0) return -1;
    if (out) *out = s->data[--s->top];
    return 0;
}