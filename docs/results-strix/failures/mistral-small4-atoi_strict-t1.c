#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    int sign = 1;
    const char *p = s;

    if (*p == '-') {
        sign = -1;
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    int value = 0;
    while (*p) {
        if (!isdigit((unsigned char)*p)) {
            return -1;
        }
        value = value * 10 + (*p - '0');
        p++;
    }

    *out = sign * value;
    return 0;
}