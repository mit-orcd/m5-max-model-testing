#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    if (expr == NULL || *expr == '\0') {
        return 0.0;
    }

    // Calculate stack size needed (worst case: all numbers)
    size_t len = strlen(expr);
    double *stack = (double *)malloc((len / 2 + 2) * sizeof(double));
    if (stack == NULL) {
        return 0.0;
    }

    size_t top = 0;
    char *token = strtok((char *)expr, " ");

    while (token != NULL) {
        if (isdigit((unsigned char)token[0])) {
            stack[top++] = atof(token);
        } else {
            double b = stack[--top];
            double a = stack[--top];
            double result = 0.0;
            switch (token[0]) {
                case '+': result = a + b; break;
                case '-': result = a - b; break;
                case '*': result = a * b; break;
                case '/': result = a / b; break;
            }
            stack[top++] = result;
        }
        token = strtok(NULL, " ");
    }

    double result = stack[top - 1];
    free(stack);
    return result;
}