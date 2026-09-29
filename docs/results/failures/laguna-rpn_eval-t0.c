#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[100];
    int top = -1;
    const char *token = strtok((char *)expr, " ");
    
    while (token) {
        if (isdigit(token[0])) {
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
    
    return stack[top];
}