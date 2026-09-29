#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

double rpn_eval(const char *expr) {
    if (!expr) return 0.0;

    double *stack = NULL;
    int capacity = 0;
    int top = -1;

    const char *ptr = expr;
    
    while (*ptr) {
        while (*ptr && isspace((unsigned char)*ptr)) {
            ptr++;
        }
        if (*ptr == '\0') break;

        char *end;
        double val = strtod(ptr, &end);

        if (end == ptr) {
            // Operator
            char op = *ptr;
            ptr++;

            if (top < 1) {
                free(stack);
                return 0.0;
            }

            double b = stack[top--];
            double a = stack[top];
            double res = 0.0;

            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: break;
            }

            if (top + 1 >= capacity) {
                capacity = capacity ? capacity * 2 : 4;
                stack = realloc(stack, capacity * sizeof(double));
            }
            stack[++top] = res;
        } else {
            // Number
            if (top + 1 >= capacity) {
                capacity = capacity ? capacity * 2 : 4;
                stack = realloc(stack, capacity * sizeof(double));
            }
            stack[++top] = val;
            ptr = end;
        }
    }

    double result = (top >= 0) ? stack[top] : 0.0;
    free(stack);
    return result;
}