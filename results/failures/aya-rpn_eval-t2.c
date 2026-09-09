#include <stdio.h>
#include <stdbool.h>

double rpn_eval(const char *expr) {
    double stack[10];
    int top = 0;
    bool is_operator[256];
    int prec[256];
    int assoc[256];
    double tmp, a, b;
    int i, j, k;

    for (i = 0; i < 256; i++) {
        is_operator[i] = false;
        prec[i] = 0;
        assoc[i] = 0;
    }

    is_operator['+'] = true;
    is_operator['-'] = true;
    is_operator['*'] = true;
    is_operator['/'] = true;

    prec['+'] = 1;
    prec['-'] = 1;
    prec['*'] = 2;
    prec['/'] = 2;

    assoc['+'] = 1;
    assoc['-'] = 1;
    assoc['*'] = 2;
    assoc['/'] = 2;

    while (*expr) {
        if (isalpha(*expr) || *expr == '-') {
            double val = 0;
            while (isalpha(*expr) || *expr == '-') {
                val = val * 10 + (*expr - '0');
                expr++;
            }
            stack[top++] = val;
        } else if (is_operator[*expr]) {
            tmp = stack[--top];
            a = stack[--top];
            b = stack[--top];

            if (*expr == '/' && b == 0) {
                return 0;
            }

            switch (*expr) {
                case '+': tmp = a + b; break;
                case '-': tmp = a - b; break;
                case '*': tmp = a * b; break;
                case '/': tmp = a / b; break;
            }

            stack[top++] = tmp;
        }
        expr++;
    }

    return stack[0];
}