#include <stddef.h>

typedef struct {
    int data[16];
    int top;
} Stack;

static void stack_init(Stack *s) {
    s->top = -1;
}

static int stack_push(Stack *s, int v) {
    if (s->top == 15) {
        return -1; // stack full
    }
    s->data[++s->top] = v;
    return 0;
}

static int stack_pop(Stack *s, int *out) {
    if (s->top == -1) {
        return -1; // stack empty
    }
    *out = s->data[s->top--];
    return 0;
}