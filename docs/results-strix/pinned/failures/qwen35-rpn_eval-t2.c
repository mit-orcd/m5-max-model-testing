#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int top = 0;
    const char *p = expr;
    char *endptr;

    while (*p) {
        while (isspace((unsigned char)*p)) {
            p++;
        }
        if (*p == '\0') break;

        double val = strtod(p, &endptr);
        if (endptr == p) {
            // It's an operator
            char op = *p;
            if (op == '+' || op == '-' || op == '*' || op == '/') {
                if (top < 2) return 0.0; // Should not happen per valid input guarantee
                double b = stack[--top];
                double a = stack[--top];
                switch (op) {
                    case '+': stack[top++] = a + b; break;
                    case '-': stack[top++] = a - b; break;
                    case '*': stack[top++] = a * b; break;
                    case '/': stack[top++] = a / b; break;
                }
            }
            p = endptr;
        } else {
            // It's a number
            stack[top++] = val;
            p = endptr;
        }
    }

    return stack[0];
}