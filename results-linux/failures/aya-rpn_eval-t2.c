#include <stdio.h>
#include <stdlib.h>

double rpn_eval(const char *expr) {
    int i, num1, num2;
    double result;
    char op;

    i = 0;
    num1 = strtol(expr + i, &i, 10);
    while (i < strlen(expr) && expr[i] != ' ') {
        op = expr[i++];
        num2 = strtol(expr + i, &i, 10);

        switch (op) {
            case '+':
                result = num1 + num2;
                break;
            case '-':
                result = num1 - num2;
                break;
            case '*':
                result = num1 * num2;
                break;
            case '/':
                result = num1 / num2;
                break;
        }

        num1 = result;
    }

    return result;
}