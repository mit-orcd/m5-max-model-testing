#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[256];
    int top = 0;
    char *copy = strdup(expr);
    char *token = strtok(copy, " ");

    while (token != NULL) {
        if (strcmp(token, "+") == 0) {
            double b = stack[--top];
            double a = stack[--top];
            stack[top++] = a + b;
        } else if (strcmp(token, "-") == 0) {
            double b = stack[--top];
            double a = stack[--top];
            stack[top++] = a - b;
        } else if (strcmp(token, "*") == 0) {
            double b = stack[--top];
            double a = stack[--top];
            stack[top++] = a * b;
        } else if (strcmp(token, "/") == 0) {
            double b = stack[--top];
            double a = stack[--top];
            stack[top++] = a / b;
        } else {
            stack[top++] = atof(token);
        }
        token = strtok(NULL, " ");
    }

    free(copy);
    return stack[0];
}