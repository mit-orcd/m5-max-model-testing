#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    // Trim leading whitespace
    char *p = s;
    while (isspace((unsigned char)*p)) {
        p++;
    }
    memmove(s, p, strlen(p) + 1);

    // Trim trailing whitespace
    p = s + strlen(s) - 1;
    while (p >= s && isspace((unsigned char)*p)) {
        p--;
    }
    *(p + 1) = '\0';
}