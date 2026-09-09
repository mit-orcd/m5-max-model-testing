
#include <stdio.h>
typedef struct { int data[16]; int top; } Stack;
void stack_init(Stack *s); int stack_push(Stack *s, int v); int stack_pop(Stack *s, int *out);
static int fails = 0;
int main(void) {
    Stack s; int v = -999; stack_init(&s);
    if (stack_pop(&s, &v) != -1) { printf("FAIL pop empty\n"); fails++; }
    for (int i = 0; i < 16; i++) if (stack_push(&s, i) != 0) { printf("FAIL push %d\n", i); fails++; }
    if (stack_push(&s, 16) != -1) { printf("FAIL push full\n"); fails++; }
    for (int i = 15; i >= 0; i--) {
        if (stack_pop(&s, &v) != 0 || v != i) { printf("FAIL pop got %d want %d\n", v, i); fails++; }
    }
    if (stack_pop(&s, &v) != -1) { printf("FAIL pop empty2\n"); fails++; }
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

