#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static int is_number(const char *s) {
    return *s >= '0' && *s <= '9';
}

double rpn_eval(const char *expr) {
    double *stack = malloc(64 * sizeof(double));
    int sp = 0;

    const char *p = expr;
    while (*p) {
        while (*p == ' ') p++;

        if (*p == '\0') break;

        char op = *p;

        if (is_number(p)) {
            double val = strtod(p, (char **)&p);
            if (sp == 64) {
                stack = realloc(stack, 2 * 64 * sizeof(double));
            }
            stack[sp++] = val;
        } else if (op == '+' || op == '-' || op == '*' || op == '/') {
            double b = stack[--sp];
            double a = stack[--sp];
            switch (op) {
                case '+': stack[sp++] = a + b; break;
                case '-': stack[sp++] = a - b; break;
                case '*': stack[sp++] = a * b; break;
                default:  stack[sp++] = a / b; break;
            }
            p += 2;
        } else {
            p += 2;
        }
    }

    double result = stack[--sp];
    free(stack);
    return result;
}