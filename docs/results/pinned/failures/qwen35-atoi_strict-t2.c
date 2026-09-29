#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    if (*p == '\0') {
        return -1;
    }

    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
        if (*p == '\0') {
            return -1;
        }
    } else if (*p == '+') {
        p++;
        if (*p == '\0') {
            return -1;
        }
    }

    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    while (isdigit((unsigned char)*p)) {
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    *out = sign * (p - s - (s[0] == '-' || s[0] == '+') ? 1 : 0);
    return 0;
}