#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[64];
    int top = -1;
    char *expr_copy = strdup(expr);
    char *token = strtok(expr_copy, " ");
    
    while (token != NULL) {
        if (isdigit(token[0])) {
            stack[++top] = atof(token);
        } else {
            double b = stack[top--];
            double a = stack[top--];
            
            switch (token[0]) {
                case '+': stack[top] = a + b; break;
                case '-': stack[top] = a - b; break;
                case '*': stack[top] = a * b; break;
                case '/': stack[top] = a / b; break;
            }
        }
        token = strtok(NULL, " ");
    }
    
    free(expr_copy);
    return stack[top];
}