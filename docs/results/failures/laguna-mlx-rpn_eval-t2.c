#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double rpn_eval(const char *expr) {
    double stack[1000];
    int top = -1;
    char *token = strtok((char *)expr, " ");
    
    while (token != NULL) {
        if (strchr("+-*/", token[0])) {
            double b = stack[top--];
            double a = stack[top--];
            switch (token[0]) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top] = a - b; break;
                case '*': stack[++top] = a * b; break;
                case '/': stack[++top] = a / b; break;
            }
        } else {
            stack[++top] = atof(token);
        }
        token = strtok(NULL, " ");
    }
    
    return stack[top];
}