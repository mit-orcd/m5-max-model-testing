#include <stddef.h>

static int match(const char *p, const char *s) {
    while (*p) {
        if (*p == '*') {
            if (match(p + 1, s)) {
                return 1;
            }
            if (*s) {
                p++;
                s++;
            }
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else {
            return 0;
        }
    }
    return *s == '\0';
}

int glob_match(const char *pat, const char *str) {
    return match(pat, str);
}