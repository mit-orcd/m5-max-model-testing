#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

double rpn_eval(const char *expr) {
    double stack[100];
    int top = 0;
    for (const char *ptr = expr; *ptr; ++ptr) {
        if (isdigit(*ptr)) {
            double num = strtod(ptr, &ptr);
            stack[top++] = num;
        } else {
            double b = stack[--top];
            double a = stack[--top];
            switch (*ptr) {
                case '+': stack[top++] = a + b; break;
                case '-': stack[top++] = a - b; break;
                case '*': stack[top++] = a * b; break;
                case '/': stack[top++] = a / b; break;
            }
        }
    }
    return stack[top];
}