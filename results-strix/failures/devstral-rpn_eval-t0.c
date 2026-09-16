#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    char *stack = malloc(strlen(expr) * sizeof(double));
    if (!stack) return NAN;
    double *top = stack;
    const char *p = expr;
    while (*p) {
        if (isspace(*p)) { p++; continue; }
        if (isdigit(*p)) {
            double num = 0;
            while (isdigit(*p)) {
                num = num * 10 + (*p - '0');
                p++;
            }
            *top++ = num;
        } else {
            double b = *(--top);
            double a = *(--top);
            switch (*p++) {
                case '+': *top++ = a + b; break;
                case '-': *top++ = a - b; break;
                case '*': *top++ = a * b; break;
                case '/': *top++ = a / b; break;
            }
        }
    }
    double result = *(--top);
    free(stack);
    return result;
}