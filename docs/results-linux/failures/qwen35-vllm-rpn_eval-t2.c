#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int top = -1;
    char *buffer = malloc(strlen(expr) + 1);
    if (!buffer) return 0.0;
    
    char *ptr = (char*)expr;
    while (*ptr) {
        // Skip whitespace
        while (isspace((unsigned char)*ptr)) ptr++;
        if (!*ptr) break;
        
        // Extract token
        int i = 0;
        while (*ptr && !isspace((unsigned char)*ptr)) {
            if (i < 1023) buffer[i++] = *ptr++;
            else { ptr++; break; }
        }
        buffer[i] = '\0';
        
        double val;
        int is_op = 0;
        
        if (strlen(buffer) == 1) {
            char op = buffer[0];
            if (op == '+') { is_op = 1; }
            else if (op == '-') { is_op = 1; }
            else if (op == '*') { is_op = 1; }
            else if (op == '/') { is_op = 1; }
        }
        
        if (is_op) {
            if (top < 1) break; // Should not happen per valid input
            double b = stack[top--];
            double a = stack[top];
            
            switch (op) {
                case '+': val = a + b; break;
                case '-': val = a - b; break;
                case '*': val = a * b; break;
                case '/': val = a / b; break;
                default: val = 0.0; break;
            }
            stack[++top] = val;
        } else {
            val = atof(buffer);
            stack[++top] = val;
        }
    }
    
    free(buffer);
    return (top >= 0) ? stack[top] : 0.0;
}