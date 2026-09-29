#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    char *copy = strdup(expr);
    char *token = strtok(copy, " ");
    double stack[256];
    int stack_top = 0;

    while (token != NULL) {
        if (isdigit(token[0])) {
            stack[stack_top++] = strtod(token, NULL);
        } else {
            double a = stack[--stack_top];
            double b = stack[--stack_top];
            switch (token[0]) {
                case '+': stack[stack_top++] = b + a; break;
                case '-': stack[stack_top++] = b - a; break;
                case '*': stack[stack_top++] = b * a; break;
                case '/': stack[stack_top++] = b / a; break;
            }
        }
        token = strtok(NULL, " ");
    }

    free(copy);
    return stack[--stack_top];
}