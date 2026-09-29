#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    char *copy = strdup(expr);
    char *token = strtok(copy, " ");
    double stack[100];
    int top = 0;

    while (token != NULL) {
        if (isdigit(token[0])) {
            stack[top++] = strtod(token, NULL);
        } else {
            double b = stack[--top];
            double a = stack[--top];
            switch (token[0]) {
                case '+': stack[top++] = a + b; break;
                case '-': stack[top++] = a - b; break;
                case '*': stack[top++] = a * b; break;
                case '/': stack[top++] = a / b; break;
            }
        }
        token = strtok(NULL, " ");
    }

    free(copy);
    return stack[0];
}