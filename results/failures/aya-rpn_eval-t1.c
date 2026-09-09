#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double rpn_eval(const char *expr) {
    double stack[100];
    int top = -1;
    char *token;

    token = strtok(expr, " ");
    while (token != NULL) {
        if (isdigit(token[0])) {
            double num = atof(token);
            stack[++top] = num;
        } else {
            double b = stack[top--];
            double a = stack[top--];
            if (strcmp(token, "+") == 0) stack[top++] = a + b;
            else if (strcmp(token, "-") == 0) stack[top++] = a - b;
            else if (strcmp(token, "*") == 0) stack[top++] = a * b;
            else if (strcmp(token, "/") == 0) stack[top++] = a / b;
            else {
                printf("Unknown operator %s\n", token);
                exit(1);
            }
        }
        token = strtok(NULL, " ");
    }
    return stack[top];
}