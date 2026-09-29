#include <stdbool.h>

static bool match_star(const char **pat, const char **str) {
    const char *p = *pat;
    const char *s = *str;
    if (p[0] != '*' && p[0] != '\0') return false;
    while (*p == '*') p++;
    if (*p == '\0') return true;
    while (*s != '\0') {
        *pat = p;
        *str = s++;
        if (match_star(pat, str)) return true;
    }
    return false;
}

int glob_match(const char *pat, const char *str) {
    const char *p = pat, *s = str;
    while (*p != '\0') {
        switch (*p) {
            case '*':
                return match_star(&p, &s);
            case '?':
                if (*s == '\0') return 0;
                p++; s++;
                break;
            default:
                if (*p != *s) return 0;
                p++; s++;
                break;
        }
    }
    return *s == '\0';
}