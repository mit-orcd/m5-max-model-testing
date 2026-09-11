#include <ctype.h>

int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *ss = NULL;

    while (*s) {
        if (*p == '*') {
            star = p;
            ss = s;
            while (*p == '*') p++;
            if (*p == '\0') return 1;
        } else if (*p == '?' || tolower((unsigned char)*p) == tolower((unsigned char)*s)) {
            p++;
            s++;
        } else if (star) {
            s = ++ss;
            p = star + 1;
        } else {
            return 0;
        }
    }

    while (*p == '*') p++;
    return *p == '\0';
}