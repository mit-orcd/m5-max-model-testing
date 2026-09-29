#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

double rpn_eval(const char *expr) {
    double stack[100];
    int top = 0;
    bool is_operator[100];
    char token[10];

    while (expr[0] != '\0') {
        if (isspace(expr[0])) {
            expr++;
            continue;
        }
        if (isdigit(expr[0]) || expr[0] == '.') {
            double num = strtod(expr, &token);
            stack[top++] = num;
            expr = token;
        } else if (expr[0] == '+' || expr[0] == '-' || expr[0] == '*' || expr[0] == '/') {
            is_operator[top - 1] = true;
            expr++;
        } else {
            printf("Invalid token: %s\n", expr);
            return 0.0;
        }
    }

    while (top > 1) {
        if (!is_operator[top - 2]) {
            stack[top - 2] += stack[top - 1];
            top--;
        } else if (is_operator[top - 2] && (stack[top - 2] == 0 || stack[top - 1] == 0)) {
            stack[top - 2] += stack[top - 1];
            top--;
        } else {
            switch (stack[top - 2]) {
                case '+':
                    stack[top - 2] = stack[top - 1] + stack[top - 2];
                    break;
                case '-':
                    stack[top - 2] = stack[top - 1] - stack[top - 2];
                    break;
                case '*':
                    stack[top - 2] = stack[top - 1] * stack[top - 2];
                    break;
                case '/':
                    stack[top - 2] = stack[top - 1] / stack[top - 2];
                    break;
            }
            top--;
        }
    }

    return stack[0];
}