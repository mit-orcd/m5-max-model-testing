#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return -1;
    }

    int negative = 0;
    if (*s == '-') {
        s++;
        if (*s == '\0') {
            return -1;
        }
        negative = 1;
    } else if (*s == '+') {
        s++;
        if (*s == '\0') {
            return -1;
        }
    }

    if (!isdigit((unsigned char)*s)) {
        return -1;
    }

    long value = 0;
    while (isdigit((unsigned char)*s)) {
        value = value * 10 + (*s - '0');
        s++;
    }

    if (negative) {
        value = -value;
    }

    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s != '\0') {
        return -1;
    }

    *out = (int)value;
    return 0;
}