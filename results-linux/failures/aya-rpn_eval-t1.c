#include <stdio.h>
#include <stdlib.h>

double rpn_eval(const char *expr) {
    int stack[100], top = 0;
    double num;
    char op;

    while (*expr) {
        if (isdigit(*expr)) {
            num = 0;
            while (isdigit(*expr)) {
                num = num * 10 + (*expr) - '0';
                expr++;
            }
            stack[top++] = num;
        } else if ((*expr) == ' ') {
            expr++;
        } else {
            op = (*expr);
            expr++;
            if (op == '+') {
                num = stack[--top] + stack[--top];
            } else if (op == '-') {
                num = stack[--top] - stack[--top];
            } else if (op == '*') {
                num = stack[--top] * stack[--top];
            } else if (op == '/') {
                num = stack[--top] / stack[--top];
            }
            stack[top++] = num;
        }
    }
    return stack[--top];
}