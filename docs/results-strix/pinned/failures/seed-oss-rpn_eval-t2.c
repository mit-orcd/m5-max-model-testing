#include <stdlib.h>
#include <string.h>
#include <stddef.h>

double rpn_eval(const char *expr) {
    double stack[1024];
    int stack_top = 0;

    size_t len = strlen(expr);
    char *expr_copy = malloc(len + 1);
    memcpy(expr_copy, expr, len + 1);

    char *token = strtok(expr_copy, " ");
    while (token != NULL) {
        if (token[0] == '+' || token[0] == '-' || token[0] == '*' || token[0] == '/') {
            double a = stack[--stack_top];
            double b = stack[--stack_top];
            double res;
            switch (token[0]) {
                case '+': res = b + a; break;
                case '-': res