#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int top = -1;
    char *copy = strdup(expr);
    char *token = strtok(copy, " ");
    while (token) {
        if (isdigit(token[0])) {
            stack[++top] = atof(token);
        } else {
            double b = stack[top--];
            double a = stack[top--];
            if (strcmp(token, "+") == 0) stack[++top] = a + b;
            else if (strcmp(token, "-") == 0) stack[++top] = a - b;
            else if (strcmp(token, "*") == 0) stack[++top] = a * b;
            else if (strcmp(token, "/") == 0) stack[++top] = a / b;
        }
        token = strtok(NULL, " ");
    }
    free(copy);
    return stack[top];
}