#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    char *stack = malloc(strlen(expr) * sizeof(double));
    char *top = stack;
    char *end;

    for (char *tok = strtok((char *)expr, " "); tok != NULL; tok = strtok(NULL, " ")) {
        if (isdigit(tok[0]) || (tok[0] == '-' && isdigit(tok[1]))) {
            *top++ = strtod(tok, &end);
        } else {
            double b = *(--top);
            double a = *(--top);
            switch (tok[0]) {
                case '+': *top++ = a + b; break;
                case '-': *top++ = a - b; break;
                case '*': *top++ = a * b; break;
                case '/': *top++ = a / b; break;
            }
        }
    }

    double result = *(--top);
    free(stack);
    return result;
}