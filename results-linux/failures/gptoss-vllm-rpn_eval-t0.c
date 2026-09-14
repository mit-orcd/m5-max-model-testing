#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    /* Count tokens to size the stack */
    size_t token_count = 1;
    for (const char *p = expr; *p; ++p) {
        if (*p == ' ') token_count++;
    }

    double *stack = (double *)malloc(token_count * sizeof(double));
    size_t sp = 0;  /* stack pointer */

    /* Duplicate the expression to use strtok */
    char *buf = strdup(expr);
    char *token = strtok(buf, " ");
    while (token) {
        if (isdigit((unsigned char)token[0])) {
            /* Integer token */
            double val = (double)strtod(token, NULL);
            stack[sp++] = val;
        } else {
            /* Operator token */
            double b = stack[--sp];
            double a = stack[--sp];
            double res = 0.0;
            switch (token[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
            }
            stack[sp++] = res;
        }
        token = strtok(NULL, " ");
    }

    double result = stack[0];
    free(stack);
    free(buf);
    return result;
}