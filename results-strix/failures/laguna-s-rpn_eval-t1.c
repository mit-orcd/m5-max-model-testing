#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STACK 1000

static double stack[MAX_STACK];
static int top = -1;

static void push(double val) {
    stack[++top] = val;
}

static double pop(void) {
    return stack[top--];
}

double rpn_eval(const char *expr) {
    top = -1;
    char *copy = strdup(expr);
    char *token = strtok(copy, " ");
    while (token != NULL) {
        if (isdigit((unsigned char)token[0])) {
            push((double)strtol(token, NULL, 10));
        } else {
            double b = pop();
            double a = pop();
            switch (token[0]) {
                case '+': push(a + b); break;
                case '-': push(a - b); break;
                case '*': push(a * b); break;
                case '/': push(a / b); break;
            }
        }
        token = strtok(NULL, " ");
    }
    free(copy);
    return pop();
}