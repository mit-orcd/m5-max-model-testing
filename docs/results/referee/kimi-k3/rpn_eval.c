#include <stdlib.h>
#include <string.h>

double rpn_eval(const char *expr) {
    double st[256];
    int n = 0;
    char buf[1024];
    strncpy(buf, expr, sizeof buf - 1);
    buf[sizeof buf - 1] = 0;
    for (char *tok = strtok(buf, " "); tok; tok = strtok(0, " ")) {
        char c = tok[0];
        if (tok[1] == 0 && (c == '+' || c == '-' || c == '*' || c == '/')) {
            double b = st[--n], a = st[--n];
            st[n++] = c == '+' ? a + b : c == '-' ? a - b : c == '*' ? a * b : a / b;
        } else {
            st[n++] = strtod(tok, 0);
        }
    }
    return st[0];
}
