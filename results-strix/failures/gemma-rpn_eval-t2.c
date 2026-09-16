#include <stdio.h>
#include <stdlib.h>
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
            if (stack_size == stack_capacity) {
                stack_capacity = (stack_capacity == 0) ? 16 : stack_capacity * 2;
                stack = realloc(stack, stack_capacity * sizeof(double));
            }
            stack[stack_size++] = atof(token);
        } else {
            double b = stack[--stack_size];
            double a = stack[--stack_size];
            if (stack_size == stack_capacity) {
                stack_capacity = (stack_capacity == 0) ? 16 : stack_capacity * 2;
                stack = realloc(stack, stack_capacity * sizeof(double));
            }
            if (stack[0] == '+') { // Note: token is char*, check first char
                // This logic is slightly flawed in structure, let's refine:
            }
            // Re-evaluating logic for operators:
            if (strcmp(token, "+") == 0) stack[stack_size++] = a + b;
            else if (strcmp(token, "-") == 0) stack[stack_size++] = a - b;
            else if (strcmp(token, "*") == 0) stack[stack_size++] = a * b;
            else if (strcmp(token, "/") == 0) stack[stack_size++] = a / b;
        }
        token = strtok(NULL, " ");
    }

    double result = stack[0];
    free(stack);
    free(expr_copy);
    return result;
}