#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (!s || !out) {
        return -1;
    }

    const char *p = s;

    if (*p == '-') {
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

    *out = (int)strtol(s, (char **)&p, 10);
    return 0;
}