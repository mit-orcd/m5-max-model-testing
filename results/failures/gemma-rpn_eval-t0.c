#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = malloc(strlen(expr) + 1);
    int top = -1;

    char *expr_copy = strdup(expr);
    char *token = strtok(expr_copy, " ");

    while (token != NULL) {
        if (isdigit(token[0])) {
            stack[++top] = atof(token);
        } else {
            double b = stack[top--];
            double a = stack[top--];
            if (token[0] == '+') stack[++top] = a + b;
            else if (token[0] == '-') stack[++top] = a - b;
            else if (token[0] == '*') stack[++top] = a * b;
            else if (token[0] == '/') stack[++top] = a / b;
        }
        token = strtok(NULL, " ");
    }

    double result = stack[0];
    free(stack);
    free(expr_copy);
    return result;
}