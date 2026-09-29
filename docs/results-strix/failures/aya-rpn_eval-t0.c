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
            double result = stack[top - 1];
            switch (stack[top - 2]) {
                case '+': result += stack[top - 1]; break;
                case '-': result -= stack[top - 1]; break;
                case '*': result *= stack[top - 1]; break;
                case '/': result /= stack[top - 1]; break;
            }
            stack[top - 2] = result;
            top--;
        }
    }

    return stack[0];
}