#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 100

double rpn_eval(const char *expr) {
    double stack[MAX_STACK];
    int top = -1;

    char *expr_copy = strdup(expr);
    char *token = strtok(expr_copy, " ");

    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            double val = atof(token);
            stack[++top] = val;
        } else {
            double b = stack[top--];
            double a = stack[top--];
            switch (token[0]) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top] = a - b; break;
                case '*': stack[++top] = a * b; break;
                case '/': stack[++top] = a / b; break;
            }
        }
        token = strtok(NULL, " ");
    }

    free(expr_copy);
    return stack[top];
}