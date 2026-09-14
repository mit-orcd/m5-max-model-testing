#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MAX_STACK 128

typedef struct {
    double items[MAX_STACK];
    int top;
} Stack;

void stack_init(Stack *s) {
    s->top = -1;
}

int stack_push(Stack *s, double val) {
    if (s->top >= MAX_STACK - 1) return 0;
    s->items[++(s->top)] = val;
    return 1;
}

int stack_pop(Stack *s, double *val) {
    if (s->top < 0) return 0;
    *val = s->items[(s->top)--];
    return 1;
}

int stack_is_empty(Stack *s) {
    return s->top < 0;
}

double rpn_eval(const char *expr) {
    Stack stack;
    stack_init(&stack);

    char *token = strtok((char *)expr, " ");
    while (token != NULL) {
        // Check if token is an operator
        if (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/') {
            double b, a;
            if (stack_pop(&stack, &b) && stack_pop(&stack, &a)) {
                switch (token[0]) {
                    case '+': stack_push(&stack, a + b); break;
                    case '-': stack_push(&stack, a - b); break;
                    case '*': stack_push(&stack, a * b); break;
                    case '/': stack_push(&stack, a / b); break;
                }
            }
        } else {
            // Parse integer
            char *endptr;
            double val = strtod(token, &endptr);
            if (endptr != token) {
                stack_push(&stack, val);
            }
        }
        token = strtok(NULL, " ");
    }

    double result;
    if (stack_pop(&stack, &result) && stack_is_empty(&stack)) {
        return result;
    }
    
    // If stack is not empty or pop failed (invalid input), though problem states valid input
    return NAN;
}