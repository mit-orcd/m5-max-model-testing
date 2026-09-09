#include <stddef.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star_p = NULL;
    const char *star_s = NULL;

    while (*s != '\0') {
        if (*p == '*') {
            star_p = p++;
            star_s = s;
            while (*star_p == '*') star_p++;
            continue;
        }
        if (*p != '\0' && (*p == '?' || *p == *s)) {
            p++;
            s++;
            continue;
        }
        if (star_p != NULL) {
            p = star_p + 1;
            s = ++star_s;
            continue;
        }
        return 0;
    }
    while (*p == '*') p++;
    return *p == '\0';
}