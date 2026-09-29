#include <stdio.h>
#include <string.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat, *s = str;
    while (*p) {
        if (*p == '*') {
            while (*s) {
                if (glob_match(p + 1, s)) return 1;
                s++;
            }
            return 0;
        } else if (*p == '?') {
            if (!*s) return 0;
            s++;
        } else {
            if (*p != *s) return 0;
            s++;
        }
        p++;
    }
    return !*s;
}