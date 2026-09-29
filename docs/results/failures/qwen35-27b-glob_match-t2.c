#include <string.h>
#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *s_match = NULL;

    while (s && *p) {
        if (*p == '*') {
            star = p;
            s_match = s;
            p++;
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else {
            if (star) {
                p = star + 1;
                s = ++s_match;
            } else {
                return 0;
            }
        }
    }

    while (p && *p == '*') {
        p++;
    }

    return !*p;
}