#include <ctype.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *ss = NULL;

    while (*s) {
        if (*p == '?') {
            if (*s && *p != '\0') {
                p++;
                s++;
            } else {
                return 0;
            }
        } else if (*p == '*') {
            star = p;
            ss = s;
            p++;
        } else if (*p == *s) {
            p++;
            s++;
        } else if (star) {
            p = star + 1;
            ss++;
            s = ss;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}