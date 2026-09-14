#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

double rpn_eval(const char *expr) {
    double stack[256];
    int top = 0;
    const char *p = expr;
    char *endptr;
    long val;

    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;

        val = strtol(p, &endptr, 10);
        if (endptr != p) {
            if (top < 2) return 0.0; // Should not happen on valid input
            top--;
            double b = stack[top];
            double a = stack[top - 1];
            double res = 0.0;
            char op = *endptr;
            while (isspace((unsigned char)*endptr)) endptr++;

            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: break;
            }
            stack[top - 1] = res;
            p = endptr;
        } else {
            stack[top++] = val;
            p = endptr;
        }
    }
    return top > 0 ? stack[0] : 0.0;
}