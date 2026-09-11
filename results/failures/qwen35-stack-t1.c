#include <limits.h>

typedef struct {
    int data[16];
    int top;
} Stack;

void stack_init(Stack *s) {
    s->top = 0;
}

int stack_push(Stack *s, int v) {
    if (s->top >= 16) {
        return -1;
    }
    s->data[s->top] = v;
    s->top++;
    return 0;
}

int stack_pop(Stack *s, int *out) {
    if (s->top <= 0) {
        return -1;
    }
    s->top--;
    if (out != NULL) {
        *out = s->data[s->top];
    }
    return 0;
}