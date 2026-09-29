#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[256];
    int top = -1;
    char *token;
    char *expr_copy = strdup(expr);
    char *saveptr;

    token = strtok_r(expr_copy, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "+") == 0) {
            double b = stack[top--];
            double a = stack[top--];
            stack[++top] = a + b;
        } else if (strcmp(token, "-") == 0) {
            double b = stack[top--];
            double a = stack[top--];
            stack[++top] = a - b;
        } else if (strcmp(token, "*") == 0) {
            double b = stack[top--];
            double a = stack[top--];
            stack[++top] = a * b;
        } else if (strcmp(token, "/") == 0) {
            double b = stack[top--];
            double a = stack[top--];
            stack[++top] = a / b;
        } else {
            stack[++top] = atof(token);
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    free(expr_copy);
    return stack[top];
}