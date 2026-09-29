#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = NULL;
    size_t stack_size = 0;
    size_t stack_capacity = 0;

    char *expr_copy = strdup(expr);
    char *token = strtok(expr_copy, " ");

    while (token != NULL) {
        if (isdigit(token[0])) {
            double val = atof(token);
            if (stack_size == stack_capacity) {
                stack_capacity = (stack_capacity == 0) ? 16 : stack_capacity * 2;
                stack = realloc(stack, stack_capacity * sizeof(double));
            }
            stack[stack_size++] = val;
        } else {
            double b = stack[--stack_size];
            double a = stack[--stack_size];
            double res = 0;

            if (token[0] == '+') res = a + b;
            else if (token[0] == '-') res = a - b;
            else if (token[0] == '*') res = a * b;
            else if (token[0] == '/') res = a / b;

            if (stack_size == stack_capacity) {
                stack_capacity = (stack_capacity == 0) ? 16 : stack_capacity * 2;
                stack = realloc(stack, stack_capacity * sizeof(double));
            }
            stack[stack_size++] = res;
        }
        token = strtok(NULL, " ");
    }

    double result = stack[0];
    free(stack);
    free(expr_copy);
    return result;
}