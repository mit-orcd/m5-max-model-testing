#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    char *buf = strdup(expr);
    char *token;
    char *saveptr = NULL;
    double stack[100];
    int sp = 0;

    while ((token = strtok_r(buf, " ", &saveptr)) != NULL) {
        if (isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))) {
            stack[sp++] = atof(token);
        } else {
            double b = stack[--sp];
            double a = stack[--sp];
            switch (token[0]) {
                case '+': stack[sp++] = a + b; break;
                case '-': stack[sp++] = a - b; break;
                case '*': stack[sp++] = a * b; break;
                case '/': stack[sp++] = a / b; break;
            }
        }
        buf = NULL;
    }

    free(buf);
    return stack[0];
}