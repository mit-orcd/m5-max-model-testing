#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 64

typedef struct {
    double data[MAX_STACK_SIZE];
    int top;
} DoubleStack;

void stack_init(DoubleStack *s) {
    s->top = -1;
}

bool stack_push(DoubleStack *s, double val) {
    if (s->top >= MAX_STACK_SIZE - 1) {
        return false;
    }
    s->data[++s->top] = val;
    return true;
}

bool stack_pop(DoubleStack *s, double *val) {
    if (s->top < 0) {
        return false;
    }
    *val = s->data[s->top--];
    return true;
}

bool stack_peek(DoubleStack *s, double *val) {
    if (s->top < 0) {
        return false;
    }
    *val = s->data[s->top];
    return true;
}

bool is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/';
}

double rpn_eval(const char *expr) {
    DoubleStack stack;
    stack_init(&stack);

    const char *p = expr;
    char *endptr;
    double num;

    while (*p != '\0') {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }

        if (is_operator(*p)) {
            double b, a;
            if (!stack_pop(&stack, &b) || !stack_pop(&stack, &a)) {
                return NAN;
            }

            switch (*p) {
                case '+': 
                    stack_push(&stack, a + b); 
                    break;
                case '-': 
                    stack_push(&stack, a - b); 
                    break;
                case '*': 
                    stack_push(&stack, a * b); 
                    break;
                case '/': 
                    if (b == 0) {
                        return NAN;
                    }
                    stack_push(&stack, a / b); 
                    break;
            }
            p++;
        } else {
            char *next_p;
            num = strtod(p, &next_p);
            if (p == next_p) {
                return NAN;
            }
            if (!stack_push(&stack, num)) {
                return NAN;
            }
            p = next_p;
        }
    }

    if (stack.top != 0) {
        return NAN;
    }

    double result;
    if (!stack_pop(&stack, &result)) {
        return NAN;
    }

    return result;
}