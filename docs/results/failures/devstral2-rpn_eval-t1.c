#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    char *stack[100];
    int sp = 0;
    char *token = strtok((char *)expr, " ");
    while (token) {
        if (isdigit(token[0]) || (token[0] == '-' && strlen(token) > 1 && isdigit(token[1]))) {
            stack[sp++] = token;
        } else {
            double b = atof(stack[--sp]);
            double a = atof(stack[--sp]);
            double res;
            switch (token[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: res = 0; // Should not happen
            }
            char buf[50];
            sprintf(buf, "%.15g", res);
            stack[sp++] = strdup(buf);
        }
        token = strtok(NULL, " ");
    }
    return atof(stack[--sp]);
}