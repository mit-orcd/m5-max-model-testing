#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

double rpn_eval(const char *expr) {
    char *stack = malloc(strlen(expr));
    char *top = stack;
    const char *p = expr;

    while (*p) {
        while (isspace(*p)) p++;
        if (*p == '\0') break;

        if (isdigit(*p)) {
            double num = 0;
            while (isdigit(*p)) {
                num = num * 10 + (*p - '0');
                p++;
            }
            *top++ = (char)num;
        } else {
            double b = (double)(*(top-1));
            double a = (double)(*(top-2));
            char op = *p++;
            double res;
            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
            }
            top -= 2;
            *top = (char)res;
        }
    }

    double result = (double)(*stack);
    free(stack);
    return result;
}