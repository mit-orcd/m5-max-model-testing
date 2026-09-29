#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    /* Maximum number of elements on the stack.
       The input is always valid, so 128 is more than enough. */
    double stack[128];
    int top = 0;

    /* strtok modifies the string, so make a copy. */
    char *buf = strdup(expr);
    if (!buf) return 0.0;  /* allocation failure, shouldn't happen */

    char *tok = strtok(buf, " ");
    while (tok) {
        if (isdigit((unsigned char)tok[0])) {
            /* Operand: convert to double and push onto the stack. */
            double val = strtod(tok, NULL);
            stack[top++] = val;
        } else {
            /* Operator: pop two operands, apply the operation, push the result. */
            double b = stack[--top];
            double a = stack[--top];
            double res = 0.0;
            switch (tok[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
            }
            stack[top++] = res;
        }
        tok = strtok(NULL, " ");
    }

    double result = stack[0];
    free(buf);
    return result;
}