#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = malloc(strlen(expr) + 1);
    int top = -1;

    const char *p = expr;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        if (isdigit((unsigned char)*p)) {
            stack[++top] = strtod(p, (char**)&p);
        } else {
            double b = stack[top--];
            double a = stack[top];
            
            if (*p == '+') stack[top] = a + b;
            else if (*p == '-') stack[top] = a - b;
            else if (*p == '*') stack[top] = a * b;
            else if (*p == '/') stack[top] = a / b;
            
            p++;
        }
    }

    double res = stack[0];
    free(stack);
    return res;
}