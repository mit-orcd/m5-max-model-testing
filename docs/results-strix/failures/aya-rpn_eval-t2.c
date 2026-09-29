#include <stdio.h>
#include <stdlib.h>

double rpn_eval(const char *expr) {
    int stack[100], top = 0;
    double num;

    while (*expr) {
        if (isdigit(expr[0]) || expr[0] == '.') {
            num = strtod(expr, &expr);
            stack[top++] = num;
        } else {
            double b = stack[--top];
            double a = stack[--top];

            switch (expr[0]) {
                case '+': stack[top++] = a + b; break;
                case '-': stack[top++] = a - b; break;
                case '*': stack[top++] = a * b; break;
                case '/': stack[top++] = a / b; break;
            }

            expr++;
        }
    }

    return stack[top - 1];
}