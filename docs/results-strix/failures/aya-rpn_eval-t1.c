#include <math.h>
#include <stdio.h>
#include <stdlib.h>

double rpn_eval(const char *expr) {
    int stack[100];
    int top = 0;
    double num;
    char op;

    while (*expr) {
        if (isdigit(expr[0])) {
            num = atof(expr);
            expr = expr + strspn(expr, "0123456789.");
            stack[top++] = num;
        } else if (op = expr[0], op == '+' || op == '-' || op == '*' || op == '/') {
            double b = stack[--top];
            double a = stack[--top];
            switch (op) {
                case '+': stack[top++] = a + b; break;
                case '-': stack[top++] = a - b; break;
                case '*': stack[top++] = a * b; break;
                case '/': stack[top++] = a / b; break;
            }
        }
        expr++;
    }
    return stack[--top];
}