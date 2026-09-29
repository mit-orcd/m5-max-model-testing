typedef struct {
    int data[16];
    int top;
} Stack;

void stack_init(Stack *s) {
    if (s != NULL) {
        s->top = -1;
    }
}

int stack_push(Stack *s, int v) {
    if (s == NULL || s->top == 15) {
        return -1;
    }
    s->data[++s->top] = v;
    return 0;
}

int stack_pop(Stack *s, int *out) {
    if (s == NULL || out == NULL || s->top < 0) {
        return -1;
    }
    *out = s->data[s->top--];
    return 0;
}