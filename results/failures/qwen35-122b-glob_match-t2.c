#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star_pat = NULL;
    const char *star_str = NULL;

    while (*p != '\0' || *s != '\0') {
        if (*p == '*') {
            star_pat = p + 1;
            star_str = s;
            p++;
        } else if (*p == *s || (*p == '?' && *s != '\0')) {
            p++;
            s++;
        } else if (star_pat != NULL) {
            p = star_pat;
            s = ++star_str;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return (*p == '\0');
}