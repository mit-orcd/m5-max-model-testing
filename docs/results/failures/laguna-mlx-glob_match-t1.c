#include <stdio.h>

static int match(const char *pat, const char *str) {
    const char *p = pat, *s = str;
    const char *star = NULL;
    const char *sstar = NULL;

    while (*s) {
        if (*p == '?') {
            p++;
            s++;
        } else if (*p == '*') {
            star = p++;
            sstar = s;
        } else if (*p == *s) {
            p++;
            s++;
        else {
            if (!star) return 0;
            p = star + 1;
            s = ++sstar;
        }
    }

    while (*p == '*') p++;

    return *p == '\0' && *s == '\0';
}

int glob_match(const char *pat, const char *str) {
    return match(pat, str);
}