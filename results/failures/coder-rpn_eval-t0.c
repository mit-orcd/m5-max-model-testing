#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double stack[1000];
    int top = -1;
    double a, b;
    
    while (*expr) {
        while (isspace(*expr)) {
            expr++;
        }
        
        if (isdigit(*expr)) {
            double num = 0;
            while (isdigit(*public) || *expr == '.') {
                if (*expr == '.') {
                    expr++;
                    break;
                }
                num = num * 10 + (*expr - '0');
                expr++;
            }
            stack[++top] = num;
        } else {
            b = stack[top--];
            a = stack[top--];
            switch (*expr) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top] = a - b; break;
                case '*': stack[++top] = a * b; break;
                case '/': stack[++top] = a / b; break;
            }
        }
        expr++;
    }
    
    return stack[0];
}