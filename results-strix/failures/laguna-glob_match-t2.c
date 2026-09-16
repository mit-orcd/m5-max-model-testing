#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    while (*pat) {
        if (pat[1] == '*') {
            const char *p = pat;
            const char *s = str;
            while (*p == '*' && *(p + 1) == *s) {
                p++;
                s++;
            }
            if (*p != '*') return 0;
            pat = p;
            str = s;
            continue;
        }
        if (*pat == '?') {
            if (!*str) return 0;
            pat++;
            str++;
            continue;
        }
        if (*pat != *str) return 0;
        pat++;
        str++;
    }
    return *str == '\0';
}