#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int top = -1;
    
    const char *ptr = expr;
    
    while (*ptr) {
        // Skip whitespace
        while (*ptr == ' ')
            ptr++;
        
        if (*ptr == '\0')
            break;
        
        // Check if it's a number
        if (isdigit(*ptr) || *ptr == '.' || *ptr == '-') {
            // Parse number
            char num_str[64];
            int i = 0;
            while (*ptr && (*ptr != ' ')) {
                if (i < 63)
                    num_str[i++] = *ptr;
                ptr++;
            }
            num_str[i] = '\0';
            stack[++top] = atof(num_str);
        } else {
            // It's an operator
            char op = *ptr;
            ptr++;
            
            double b = stack[top--];
            double a = stack[top--];
            
            switch (op) {
                case '+':
                    stack[++top] = a + b;
                    break;
                case '-':
                    stack[++top] = a - b;
                    break;
                case '*':
                    stack[++top] = a * b;
                    break;
                case '/':
                    stack[++top] = a / b;
                    break;
            }
        }
    }
    
    return stack[0];
}