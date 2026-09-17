#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr)
{
    size_t len = strlen(expr);
    char *buf = malloc(len + 1);
    if (!buf) return 0.0;               /* allocation failure (should not happen) */
    memcpy(buf, expr, len + 1);

    /* worst‑case stack size: one element per character */
    double *stack = malloc((len + 1) * sizeof(double));
    if (!stack) {
        free(buf);
        return 0.0;
    }
    size_t sp = 0;                      /* stack pointer */

    char *saveptr = NULL;
    char *tok = strtok_r(buf, " \t\r\n", &saveptr);
    while (tok) {
        if (isdigit((unsigned char)tok[0])) {
            /* token is a non‑negative integer */
            double val = strtod(tok, NULL);
            stack[sp++] = val;
        } else {
            /* token is an operator: pop two operands */
            double b = stack[--sp];
            double a = stack[--sp];
            double res = 0.0;

            switch (tok[0]) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default:  /* should not occur for valid input */ break;
            }
            stack[sp++] = res;
        }
        tok = strtok_r(NULL, " \t\r\n", &saveptr);
    }

    double result = stack[0];
    free(stack);
    free(buf);
    return result;
}