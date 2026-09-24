#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    double *data;
    size_t size;
    size_t capacity;
} Stack;

static void stack_init(Stack *s, size_t initial_capacity) {
    s->data = malloc(initial_capacity * sizeof(double));
    s->size = 0;
    s->capacity = initial_capacity;
}

static void stack_push(Stack *s, double value) {
    if (s->size == s->capacity) {
        s->capacity *= 2;
        s->data = realloc(s->data, s->capacity * sizeof(double));
    }
    s->data[s->size++] = value;
}

static double stack_pop(Stack *s) {
    return s->data[--s->size];
}

static double stack_peek(Stack *s) {
    return s->data[s->size - 1];
}

static void stack_free(Stack *s) {
    free(s->data);
    s->data = NULL;
    s->size = s->capacity = 0;
}

double rpn_eval(const char *expr) {
    Stack stack;
    stack_init(&stack, 16);

    char *token = strtok((char *)expr, " ");
    while (token != NULL) {
        if (token[0] == '\0') {
            token = strtok(NULL, " ");
            continue;
        }

        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            double num = strtod(token, NULL);
            stack_push(&stack, num);
        } else {
            double b = stack_pop(&stack);
            double a = stack_pop(&stack);
            switch (token[0]) {
                case '+': stack_push(&stack, a + b); break;
                case '-': stack_push(&stack, a - b); break;
                case '*': stack_push(&stack, a * b); break;
                case '/': stack_push(&stack, a / b); break;
            }
        }
        token = strtok(NULL, " ");
    }

    double result = stack_pop(&stack);
    stack_free(&stack);
    return result;
}