#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *s = str;
    const char *p = pat;
    const char *star = NULL;
    const char *match = NULL;

    while (*s) {
        if (*p == *s) {
            p++;
            s++;
        } else if (*p == '?') {
            p++;
            s++;
        } else if (*p == '*') {
            star = p++;
            match = s;
        } else if (star) {
            p = star + 1;
            s = ++match;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}