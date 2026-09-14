#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    size_t len = strlen(expr);
    if (len == 0) return 0.0;
    
    // Allocate stack with reasonable capacity
    size_t capacity = len / 2 + 1;
    double *stack = malloc(capacity * sizeof(double));
    int top = -1;
    
    char *buf = strdup(expr);
    char *token = strtok(buf, " ");
    
    while (token != NULL) {
        // Check if token is a number
        if (isdigit((unsigned char)token[0])) {
            top++;
            stack[top] = atof(token);
        } else {
            // It's an operator
            double b = stack[top--];
            double a = stack[top--];
            switch (token[0]) {
                case '+':
                    stack[++top] = a + b;
                    break;
                case '-':
                    stack[++top] = a - b;
                    break;
                case '*':
                    stack[++top] = a * b;
                    break;
                case '/':
                    stack[++top] = a / b;
                    break;
            }
        }
        token = strtok(NULL, " ");
    }
    
    double result = stack[top];
    free(stack);
    free(buf);
    return result;
}