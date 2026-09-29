#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '\0') {
        return -1;
    }

    int negative = 0;
    if (*p == '-') {
        negative = 1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0' || !isdigit((unsigned char)*p)) {
        return -1;
    }

    int result = 0;
    while (isdigit((unsigned char)*p)) {
        result = result * 10 + (*p - '0');
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (*p != '\0') {
        return -1;
    }

    *out = negative ? -result : result;
    return 0;
}