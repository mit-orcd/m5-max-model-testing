#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') {
        return -1;
    }

    const char *current = s;
    int sign = 1;

    if (*current == '+' || *current == '-') {
        sign = (*current == '-') ? -1 : 1;
        current++;
        if (*current == '\0') {
            return -1;
        }
    }

    if (!isdigit((unsigned char)*current)) {
        return -1;
    }

    int value = 0;
    while (*current != '\0') {
        if (!isdigit((unsigned char)*current)) {
            return -1;
        }
        value = value * 10 + (*current - '0');
        current++;
    }

    *out = sign * value;
    return 0;
}