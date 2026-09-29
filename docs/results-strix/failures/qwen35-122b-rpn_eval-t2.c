#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

double rpn_eval(const char *expr) {
    if (expr == NULL || *expr == '\0') {
        return 0.0;
    }

    // Maximum stack size based on typical expression length constraints
    double stack[1024];
    int top = -1;

    char *copy = strdup(expr);
    if (copy == NULL) {
        return 0.0;
    }

    char *token = strtok(copy, " ");
    while (token != NULL) {
        if (isdigit((unsigned char)token[0]) || (token[0] == '-' && isdigit((unsigned char)token[1]))) {
            // It's a number (non-negative per spec, but handling standard parsing)
            top++;
            stack[top] = atof(token);
        } else {
            // It's an operator
            double b = stack[top--];
            double a = stack[top--];
            double result = 0.0;

            switch (token[0]) {
                case '+':
                    result = a + b;
                    break;
                case '-':
                    result = a - b;
                    break;
                case '*':
                    result = a * b;
                    break;
                case '/':
                    result = a / b;
                    break;
            }
            top++;
            stack[top] = result;
        }
        token = strtok(NULL, " ");
    }

    free(copy);
    return stack[0];
}