#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = NULL;
    size_t top = 0;
    size_t cap = 0;

    while (*expr) {
        while (isspace((unsigned char)*expr)) expr++;
        if (*expr == '\0') break;

        char *end;
        double val = strtod(expr, &end);
        if (end != expr) {
            if (top == cap) {
                cap = cap == 0 ? 16 : cap * 2;
                stack = realloc(stack, cap * sizeof(double));
            }
            stack[top++] = val;
            expr = end;
        } else {
            char op = *expr++;
            double b = stack[--top];
            double a = stack[--top];
            double res;
            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: break;
            }
            stack[top - 1] = res;
        }
    }

    double result = stack[0];
    free(stack);
    return result;
}