#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    double *stack = NULL;
    int capacity = 0;
    int top = -1;
    
    char *copy = strdup(expr);
    if (!copy) return 0.0;
    
    char *token = strtok(copy, " ");
    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '.')) {
            double val = atof(token);
            if (top + 1 >= capacity) {
                capacity = capacity == 0 ? 16 : capacity * 2;
                stack = realloc(stack, capacity * sizeof(double));
            }
            stack[++top] = val;
        } else {
            double b = stack[top--];
            double a = stack[top--];
            char op = token[0];
            
            if (op == '+') stack[++top] = a + b;
            else if (op == '-') stack[++top] = a - b;
            else if (op == '*') stack[++top] = a * b;
            else if (op == '/') stack[++top] = a / b;
        }
        token = strtok(NULL, " ");
    }
    
    double result = (top >= 0) ? stack[top] : 0.0;
    free(stack);
    free(copy);
    return result;
}