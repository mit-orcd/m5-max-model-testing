void stack_init(Stack *s) {
    s->top = -1;
}

int stack_push(Stack *s, int v) {
    if (s->top == 15) {
        return -1;
    }
    s->top++;
    s->data[s->top] = v;
    return 0;
}

int stack_pop(Stack *s, int *out) {
    if (s->top == -1) {
        return -1;
    }
    *out = s->data[s->top];
    s->top--;
    return 0;
}