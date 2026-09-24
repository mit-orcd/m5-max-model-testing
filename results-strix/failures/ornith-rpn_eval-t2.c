double rpn_eval(const char *expr) {
    double stack[256];
    int top = -1;
    const char *p = expr;

    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;

        char *end = NULL;
        double val = strtod(p, &end);
        if (end != p) {
            stack[++top] = val;
            p = end;
        } else {
            switch (*p) {
                case '+': stack[top - 1] = stack[top - 1] + stack[top]; break;
                case '-': stack[top - 1] = stack[top - 1] - stack[top]; break;
                case '*': stack[top - 1] = stack[top - 1] * stack[top]; break;
                case '/': stack[top - 1] = stack[top - 1] / stack[top]; break;
            }
            top--;
            p++;
        }
    }

    return stack[0];
}