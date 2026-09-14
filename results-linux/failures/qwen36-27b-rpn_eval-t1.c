#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    if (!expr || !*expr) return 0.0;
    
    size_t len = strlen(expr);
    size_t capacity = len / 2 + 1;
    double *stack = (double *)malloc(capacity * sizeof(double));
    size_t top = 0;
    
    const char *p = expr;
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        
        const char *start = p;
        int is_negative = 0;
        if (*p == '-') {
            is_negative = 1;
            p++;
        }
        
        if (isdigit((unsigned char)*p)) {
            double num = 0.0;
            while (isdigit((unsigned char)*p)) {
                num = num * 10.0 + (*p - '0');
                p++;
            }
            if (is_negative) num = -num;
            stack[top++] = num;
        } else {
            char op = *p;
            p++;
            double b = stack[--top];
            double a = stack[--top];
            double res;
            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: res = 0.0; break;
            }
            stack[top++] = res;
        }
    }
    
    double result = stack[0];
    free(stack);
    return result;
}