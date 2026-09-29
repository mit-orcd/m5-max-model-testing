#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double rpn_eval(const char *expr) {
    double *stack = NULL;
    int top = -1;
    int capacity = 0;
    
    const char *p = expr;
    char *end;
    
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;
        
        char *token = strtok((char*)p, " \t\n");
        if (token == NULL) break;
        
        double val = strtod(token, &end);
        
        if (*end == '\0') {
            // It's a number
            if (top + 1 >= capacity) {
                capacity = capacity == 0 ? 10 : capacity * 2;
                stack = realloc(stack, capacity * sizeof(double));
            }
            stack[++top] = val;
        } else {
            // It's an operator
            char op = *end;
            if (top < 2) {
                free(stack);
                return 0.0;
            }
            double b = stack[top--];
            double a = stack[top--];
            double res = 0.0;
            
            switch (op) {
                case '+': res = a + b; break;
                case '-': res = a - b; break;
                case '*': res = a * b; break;
                case '/': res = a / b; break;
                default: free(stack); return 0.0;
            }
            stack[++top] = res;
        }
        
        // Move pointer past the token and spaces
        p = token + strlen(token);
    }
    
    double result = (top >= 0) ? stack[top] : 0.0;
    free(stack);
    return result;
}