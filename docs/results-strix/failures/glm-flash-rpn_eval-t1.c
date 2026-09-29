#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STACK_SIZE 256

double rpn_eval(const char *expr) {
    char *token;
    char buffer[256];
    char *rest = (char *)expr;
    
    double stack[STACK_SIZE];
    int stack_ptr = 0;

    while ((token = strtok_r(rest, " ", &rest)) != NULL) {
        // Check if token is a number
        char *endptr;
        double val = strtod(token, &endptr);

        if (endptr != token) {
            // It is a number
            if (stack_ptr >= STACK_SIZE) return NAN;
            stack[stack_ptr++] = val;
        } else {
            // It is an operator
            if (stack_ptr < 2) return NAN;
            
            double b = stack[--stack_ptr];
            double a = stack[--stack_ptr];
            double res;

            switch (token[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': 
                    if (b == 0.0) return NAN;
                    res = a / b; 
                    break;
                default:
                    return NAN;
            }

            if (stack_ptr >= STACK_SIZE) return NAN;
            stack[stack_ptr++] = res;
        }
    }

    if (stack_ptr != 1) return NAN;
    return stack[0];
}