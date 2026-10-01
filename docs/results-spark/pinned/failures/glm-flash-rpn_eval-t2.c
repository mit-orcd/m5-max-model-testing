#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK_SIZE 256

typedef struct {
    double data[MAX_STACK_SIZE];
    int top;
} Stack;

void stack_init(Stack *s) {
    s->top = -1;
}

int stack_push(Stack *s, double val) {
    if (s->top >= MAX_STACK_SIZE - 1) return 0;
    s->data[++(s->top)] = val;
    return 1;
}

int stack_pop(Stack *s, double *val) {
    if (s->top < 0) return 0;
    *val = s->data[(s->top)--];
    return 1;
}

int stack_is_empty(Stack *s) {
    return s->top < 0;
}

double rpn_eval(const char *expr) {
    Stack stack;
    stack_init(&stack);

    const char *p = expr;
    char *endptr;
    char token[32];

    while (*p != '\0') {
        // Skip whitespace
        while (isspace((unsigned char)*p)) p++;

        if (*p == '\0') break;

        // Check for operator
        int is_operator = 0;
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
            is_operator = 1;
            token[0] = *p;
            token[1] = '\0';
            p++;
        } else {
            // Parse number
            char *t = token;
            while (*p != '\0' && !isspace((unsigned char)*p)) {
                *t++ = *p++;
            }
            *t = '\0';
        }

        if (is_operator) {
            double b, a;
            if (stack_pop(&stack, &b) && stack_pop(&stack, &a)) {
                switch (token[0]) {
                    case '+': stack_push(&stack, a + b); break;
                    case '-': stack_push(&stack, a - b); break;
                    case '*': stack_push(&stack, a * b); break;
                    case '/': stack_push(&stack, a / b); break;
                }
            } else {
                // Stack underflow (invalid expression)
                return NAN;
            }
        } else {
            // Parse number to double
            errno = 0;
            double val = strtod(token, &endptr);
            if (endptr == token || errno == ERANGE) {
                return NAN;
            }
            stack_push(&stack, val);
        }
    }

    double result;
    if (stack_pop(&stack, &result) && stack_is_empty(&stack)) {
        return result;
    } else {
        return NAN;
    }
}