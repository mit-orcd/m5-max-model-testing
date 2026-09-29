```c
#include <string.h>
#include <stdlib.h>

double rpn_eval(const char *expr) {
    char* expr_copy = strdup(expr);
    char* token = strtok(expr_copy, " ");
    double stack[100