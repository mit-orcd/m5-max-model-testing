#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK 256

typedef struct {
    double data[MAX_STACK];
    int top;
} Stack;

void stack_init(Stack *s) {
    s->top = -1;
}

int stack_push(Stack *s, double val) {
    if (s->top >= MAX_STACK - 1) return 0;
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

int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

double rpn_eval(const char *expr) {
    Stack stack;
    stack_init(&stack);

    char *token;
    char buffer[256];
    char *saveptr = NULL;

    // Tokenize input
    for (token = strtok_r(expr, " ", &saveptr); token != NULL; token = strtok_r(NULL, " ", &saveptr)) {
        
        if (is_operator(token[0])) {
            double b, a;
            if (stack_pop(&stack, &b) && stack_pop(&stack, &a)) {
                switch (token[0]) {
                    case '+': stack_push(&stack, a + b); break;
                    case '-': stack_push(&stack, a - b); break;
                    case '*': stack_push(&stack, a * b); break;
                    case '/': 
                        if (b == 0.0) {
                            // Division by zero is undefined, per problem constraints input is valid
                            // but returning NaN is the standard behavior for IEEE 754
                            stack_push(&stack, NAN); 
                        } else {
                            stack_push(&stack, a / b); 
                        }
                        break;
                }
            } else {
                // Malformed expression (not enough operands)
                // Per problem constraints, input is always valid.
            }
        } else {
            // Parse number
            char *endptr;
            double val = strtod(token, &endptr);
            if (endptr == token) {
                // Not a valid number
            } else {
                stack_push(&stack, val);
            }
        }
    }

    double result;
    if (stack_pop(&stack, &result) && stack_is_empty(&stack)) {
        return result;
    }

    // Malformed expression
    return NAN;
}