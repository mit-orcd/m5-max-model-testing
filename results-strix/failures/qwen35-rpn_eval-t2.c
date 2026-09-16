#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    // Dynamic stack for doubles
    double *stack = NULL;
    size_t capacity = 0;
    size_t top = 0;

    // Helper to push
    auto push = [&](double val) {
        if (top == capacity) {
            size_t new_cap = capacity == 0 ? 8 : capacity * 2;
            double *new_stack = realloc(stack, new_cap * sizeof(double));
            if (!new_stack) {
                free(stack);
                return 1; // Error
            }
            stack = new_stack;
            capacity = new_cap;
        }
        stack[top++] = val;
        return 0;
    };

    // Helper to pop
    auto pop = [&]() -> double {
        if (top == 0) return 0.0; // Should not happen on valid input
        return stack[--top];
    };

    char *buffer = strdup(expr);
    if (!buffer) {
        free(stack);
        return 0.0;
    }

    char *token = strtok(buffer, " ");
    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '.' && isdigit(token[1]))) {
            if (push(atof(token)) != 0) {
                free(buffer);
                free(stack);
                return 0.0;
            }
        } else if (strlen(token) == 1) {
            char op = token[0];
            double b = pop();
            double a = pop();
            double res = 0.0;

            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: break; // Should not happen
            }
            if (push(res) != 0) {
                free(buffer);
                free(stack);
                return 0.0;
            }
        }
        token = strtok(NULL, " ");
    }

    double result = (top > 0) ? stack[0] : 0.0;
    
    free(buffer);
    free(stack);
    return result;
}