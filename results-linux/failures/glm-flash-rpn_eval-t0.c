#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

double rpn_eval(const char *expr) {
    // Determine the maximum number of tokens based on the length of the string.
    // Each token is at least 1 character (digit or operator).
    // We allocate a buffer for the tokens.
    size_t expr_len = strlen(expr);
    size_t max_tokens = expr_len + 1;
    char *tokens = malloc(max_tokens * sizeof(char));
    if (!tokens) {
        return NAN;
    }

    // Copy the expression to a mutable buffer.
    strcpy(tokens, expr);

    // Split the expression into tokens.
    char *token;
    char *saveptr = NULL;
    double *stack = malloc(max_tokens * sizeof(double));
    if (!stack) {
        free(tokens);
        return NAN;
    }
    size_t stack_size = 0;

    // Iterate through the tokens.
    for (token = strtok_r(tokens, " ", &saveptr); token != NULL; token = strtok_r(NULL, " ", &saveptr)) {
        // Check if the token is a number.
        if (isdigit(token[0]) || token[0] == '.') {
            // Convert the token to a double.
            char *endptr;
            double num = strtod(token, &endptr);
            if (endptr == token) {
                // Invalid token.
                free(tokens);
                free(stack);
                return NAN;
            }
            // Push the number onto the stack.
            stack[stack_size++] = num;
        } else {
            // The token is an operator.
            if (stack_size < 2) {
                // Not enough operands.
                free(tokens);
                free(stack);
                return NAN;
            }
            // Pop the operands.
            double b = stack[--stack_size];
            double a = stack[--stack_size];
            double result = 0.0;
            int valid_op = 1;

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
                    if (b == 0.0) {
                        valid_op = 0;
                    } else {
                        result = a / b;
                    }
                    break;
                default:
                    valid_op = 0;
                    break;
            }

            if (!valid_op) {
                free(tokens);
                free(stack);
                return NAN;
            }

            // Push the result back onto the stack.
            stack[stack_size++] = result;
        }
    }

    // The result should be the only value left on the stack.
    if (stack_size != 1) {
        free(tokens);
        free(stack);
        return NAN;
    }

    double final_result = stack[0];
    free(tokens);
    free(stack);
    return final_result;
}