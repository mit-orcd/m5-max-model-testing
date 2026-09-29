#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    int result = 0;
    int sign = 1;
    size_t i = 0;

    if (s == NULL || *s == '\0') {
        return -1;
    }

    if (s[i] == '-') {
        sign = -1;
        i++;
        if (s[i] == '\0') {
            return -1;
        }
    }

    while (s[i] != '\0') {
        if (isdigit((unsigned char)s[i])) {
            result = result * 10 + (s[i] - '0');
            i++;
        } else {
            break;
        }
    }

    if (s[i] != '\0') {
        return -1;
    }

    *out = sign * result;
    return 0;
}