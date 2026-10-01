#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *match_s = NULL;

    while (*s) {
        if (*p == '*') {
            star = p;
            match_s = s;
            p++;
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else {
            if (star) {
                p = star + 1;
                match_s++;
                s = match_s;
            } else {
                return 0;
            }
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}