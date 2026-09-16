#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = NULL;
    size_t capacity = 0;
    size_t top = 0;

    const char *p = expr;
    while (*p) {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        if (isdigit((unsigned char)*p)) {
            if (top == capacity) {
                capacity = capacity == 0 ? 16 : capacity * 2;
                double *new_stack = realloc(stack, capacity * sizeof(double));
                if (!new_stack) { free(stack); return 0.0; }
                stack = new_stack;
            }
            char *end;
            stack[top++] = strtod(p, &end);
            p = end;
        } else {
            double b = stack[--top];
            double a = stack[--top];
            char op = *p++;
            switch (op) {
                case '+': stack[--top] = a + b; break;
                case '-': stack[--top] = a - b; break;
                case '*': stack[--top] = a * b; break;
                case '/': stack[--top] = a / b; break;
            }
        }
    }

    double result = top > 0 ? stack[top - 1] : 0.0;
    free(stack);
    return result;
}