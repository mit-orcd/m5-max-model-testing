#include <stdlib.h>
#include <string.h>
#include <stdio.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int sp = 0;
    const char *p = expr;
    while (*p) {
        while (*p == ' ') p++;
        if (*p == '\0') break;
        char *end;
        double val = strtod(p, &end);
        if (end != p) {
            stack[sp++] = val;
            p = end;
            continue;
        }
        double b = stack[--sp];
        double a = stack[--sp];
        switch (*p) {
            case '+': stack[--sp] = a + b; break;
            case '-': stack[--sp] = a - b; break;
            case '*': stack[--sp] = a * b; break;
            case '/': stack[--sp] = a / b; break;
        }
        p = end;
    }
    return stack[0];
}