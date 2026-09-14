#include <ctype.h>

int glob_match(const char *pat, const char *str) {
    const char *s = str;
    const char *p = pat;
    const char *star = NULL;
    const char *match = NULL;

    while (*s) {
        switch (*p) {
            case '*':
                star = p++;
                match = s;
                break;
            case '?':
                p++;
                s++;
                break;
            case '\0':
                return 1;
            default:
                if (*p == *s) {
                    p++;
                    s++;
                } else {
                    if (star) {
                        p = star + 1;
                        match++;
                        s = match;
                    } else {
                        return 0;
                    }
                }
                break;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}