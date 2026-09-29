```c
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    size_t len = strlen(expr);
    double *stack = (double *)malloc((len + 1) * sizeof(double));
    size_t top = 0;

    const char *p = expr;
    while (*p) {
        while (*p == ' ')
            ++p;
        if (*p == '\0')
            break;

        const char *start = p;
        while (*p && *p != ' ')
            ++p;
        size_t token_len = p - start;

        if (token_len == 1 && (*start == '+' || *start == '-' ||
                               *start == '*' || *start == '/')) {
            double b = stack[--top];
            double a = stack[--top];
            double res;
            switch (*start) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b