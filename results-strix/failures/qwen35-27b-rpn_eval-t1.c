#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STACK 1024

double rpn_eval(const char *expr) {
    double stack[MAX_STACK];
    int top = -1;
    
    char *expr_copy = strdup(expr);
    char *token = strtok(expr_copy, " ");
    
    while (token != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            stack[++top] = atof(token);
        } else {
            double b = stack[top--];
            double a = stack[top--];
            switch (token[0]) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top] = a - b; break;
                case '*': stack[++top] = a * b; break;
                case '/': stack[++top] = a / b; break;
            }
        }
        token = strtok(NULL, " ");
    }
    
    free(expr_copy);
    return stack[0];
}