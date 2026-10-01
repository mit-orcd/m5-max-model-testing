#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (*s == '\0') {
        return -1;
    }

    const char *p = s;
    int sign = 1;

    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    while (*p != '\0') {
        if (!isdigit((unsigned char)*p)) {
            return -1;
        }
        p++;
    }

    int result = 0;
    const char *digit_start = s;

    if (s[0] == '+' || s[0] == '-') {
        digit_start = s + 1;
    }

    for