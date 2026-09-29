#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    if (*p == '\0') {
        return -1;
    }

    if (*p == '-' || *p == '+') {
        p++;
        if (*p == '\0') {
            return -1;
        }
    }

    if (!isdigit((unsigned char)*p)) {
        return -1;
    }

    int sign = 1;
    if (s[0] == '-') {
        sign = -1;
    }

    long long value = 0;
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    *out = (int)(sign * value);
    return 0;
}