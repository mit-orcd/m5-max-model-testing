#include <string.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *ss = NULL;

    while (*s) {
        if (*p == '*') {
            star = p++;
            ss = s;
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else if (star) {
            p = star + 1;
            s = ss + 1;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}