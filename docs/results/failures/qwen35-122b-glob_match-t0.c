#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star_pat = NULL;
    const char *star_str = NULL;

    while (*p != '\0') {
        if (*p == '*') {
            star_pat = p;
            star_str = s;
            p++;
        } else if (*p == '?' || *p == *s) {
            if (*p == '?' && *s == '\0') {
                return 0;
            }
            p++;
            s++;
        } else if (star_pat != NULL) {
            p = star_pat + 1;
            s = ++star_str;
        } else {
            return 0;
        }
    }

    while (*s != '\0') {
        if (star_pat == NULL) {
            return 0;
        }
        star_pat++;
        s++;
    }

    return 1;
}