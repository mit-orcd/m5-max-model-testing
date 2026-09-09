double rpn_eval(const char *expr) {
    double stack[1024];
    int top = -1;

    for (const char *p = expr; *p; ) {
        while (*p == ' ') p++;
        if (!*p) break;

        char *end;
        double val = strtod(p, &end);
        if (end != p) {
            stack[++top] = val;
            p = end;
        } else {
            char op = *p;
            double b = stack[top--];
            double a = stack[top--];
            switch (op) {
                case '+': stack[++top] = a + b; break;
                case '-': stack[++top] = a - b; break;
                case '*': stack[++top] = a * b; break;
                case '/': stack[++top] = a / b; break;
            }
            p++;
        }
    }

    return stack[top];
}