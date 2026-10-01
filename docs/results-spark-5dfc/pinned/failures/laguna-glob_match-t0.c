#include <stddef.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *ss = NULL;

    while (*p) {
        if (p[1] == '*') {
            star = p;
            ss = s;
            p++;
        } else if (*p == '?') {
            if (!*s) return 0;
            p++;
            s++;
        } else if (*p == *s) {
            p++;
            s++;
        } else if (star) {
            p = star + 1;
            s = ss++;
        } else {
            return 0;
        }
    }

    while (*p == '*') p++;

    return *p == '\0';
}