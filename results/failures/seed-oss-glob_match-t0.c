#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    bool (*match_here)(const char *, const char *) = NULL;
    bool (*match_star)(const char *, const char *) = NULL;

    match_here = [=](const char *p, const char *s) {
        for (;; ++p, ++s) {
            if (*p == '\0') return *s == '\0';
            if (*p == '*') return match_star(p + 1, s);
            if (*s == '\0') return false;
            if (*p != '?' && *p != *s) return false;
            if (*s != '\0') continue;
            return *p == '\0';
        }
    };

    match_star = [=](const char *p, const char *s) {
        do {
            if (match_here(p, s)) return true;
        } while (*s++ != '\0');
        return false;
    };

    return match_here(pat, str) ? 1 : 0;
}