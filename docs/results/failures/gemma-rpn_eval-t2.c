#include <stdlib.h>
#include <string.h>

double rpn_eval(const char *expr) {
    double *stack = malloc(strlen(expr) + 1);
    int top = -1;
    char *copy = strdup(expr);
    char *token = strtok(copy, " ");

    while (token != NULL) {
        if (strlen(token) == 1 && (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/')) {
            double b = stack[top--];
            double a = stack[top--];
            if (token[0] == '+') stack[++top] = a + b;
            else if (token[0] == '-') stack[++top] = a - b;
            else if (token[0] == '*') stack[++top] = a * b;
            else if (token[0] == '/') stack[++top] = a / b;
        } else {
            stack[++top] = atof(token);
        }
        token = strtok(NULL, " ");
    }

    double result = stack[0];
    free(stack);
    free(copy);
    return result;
}