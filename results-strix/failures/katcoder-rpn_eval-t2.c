#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = NULL;
    size_t cap = 0, top = 0;

    for (const char *p = expr; *p; ) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        char *end;
        double val = strtod(p, &end);
        if (end != p) {
            if (top == cap) {
                cap = cap ? cap * 2 : 16;
                stack = realloc(stack, cap * sizeof(double));
            }
            stack[top++] = val;
            p = end;
            continue;
        }

        char op = *p++;
        double b = stack[--top];
        double a = stack[--top];
        switch (op) {
            case '+': stack[top-1] = a + b; break;
            case '-': stack[top-1] = a - b; break;
            case '*': stack[top-1] = a * b; break;
            case '/': stack[top-1] = a / b; break;
        }
    }

    double result = stack[0];
    free(stack);
    return result;
}