#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    char *stack[100];
    int top = -1;

    char *copy = strdup(expr);
    char *token = strtok(copy, " ");
    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            double num = strtod(token, NULL);
            top++;
            stack[top] = malloc(sizeof(double));
            *(double *)stack[top] = num;
        } else {
            if (top < 1) {
                free(copy);
                return 0.0;
            }
            double b = *(double *)stack[top--];
            double a = *(double *)stack[top--];
            double res = 0.0;
            switch (token[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
            }
            top++;
            stack[top] = malloc(sizeof(double));
            *(double *)stack[top] = res;
        }
        token = strtok(NULL, " ");
    }

    double result = *(double *)stack[top];
    for (int i = 0; i <= top; i++) {
        free(stack[i]);
    }
    free(copy);
    return result;
}