#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    char *stack = malloc(strlen(expr) * sizeof(double));
    if (!stack) return 0.0;

    double *top = stack;
    char *token = strtok((char *)expr, " ");
    while (token) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            double num = strtod(token, NULL);
            *(top++) = num;
        } else {
            double b = *(--top);
            double a = *(--top);
            switch (token[0]) {
                case '+': *top = a + b; break;
                case '-': *top = a - b; break;
                case '*': *top = a * b; break;
                case '/': *top = a / b; break;
            }
        }
        token = strtok(NULL, " ");
    }

    double result = stack[0];
    free(stack);
    return result;
}