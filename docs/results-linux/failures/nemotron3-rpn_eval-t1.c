#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int top = 0;
    char *token;
    char *copy = strdup(expr);
    char *saveptr;

    token = strtok_r(copy, " ", &saveptr);
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
        token = strtok_r(NULL, " ", &saveptr);
    }

    free(copy);
    return stack[top - 1];
}