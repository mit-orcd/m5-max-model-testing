#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *s = str, *p = pat;
    const char *star = NULL;
    const char *match = s;

    while (*s) {
        if (*p == '?' || *p == *s) {
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

    while (*p == '*') p++;

    return !*p;
}