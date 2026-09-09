#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

double rpn_eval(const char *expr) {
    double stack[100];
    int top = -1;
    
    const char *token = strtok(expr, " ");
    while (token != NULL) {
        if (strlen(token) == 1 && strchr("+-*/", token[0])) {
            double b = stack[top--];
            double a = stack[top--];
            switch (token[0]) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top = a - b; break;
                case '*': stack[++top] = a * b;      break;
                case '/': stack[++top] = a / b;      break;
            }
        } else {
            stack[++top] = atof(token);
        }
        token = strtok(NULL, " ");
    }
    
    return stack[0];
}