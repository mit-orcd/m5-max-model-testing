#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 256

double rpn_eval(const char *expr) {
    double stack[MAX_STACK];
    int top = -1;

    char *expr_copy = strdup(expr);
    char *token = strtok(expr_copy, " ");

    while (token != NULL) {
        if (isdigit(token[0])) {
            double val = atof(token);
            stack[++top] = val;
        } else {
            double b = stack[top--];
            double a = stack[top--];
            double res = 0.0;

            switch (token[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
            }
            stack[++top] = res;
        }
        token = strtok(NULL, " ");
    }

    double result = stack[top];
    free(expr_copy);
    return result;
}