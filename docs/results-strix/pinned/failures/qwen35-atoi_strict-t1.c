#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    if (*p == '-') {
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0' || !isdigit((unsigned char)*p)) {
        return -1;
    }

    while (isdigit((unsigned char)*p)) {
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    long val = 0;
    int negative = (s != NULL && *s == '-');
    const char *curr = s;
    if (*curr == '-' || *curr == '+') {
        curr++;
    }

    while (isdigit((unsigned char)*curr)) {
        val = val * 10 + (*curr - '0');
        curr++;
    }

    if (negative) {
        val = -val;
    }

    *out = (int)val;
    return 0;
}