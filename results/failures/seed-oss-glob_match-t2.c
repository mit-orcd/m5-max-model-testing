#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    bool (*match_here)(const char*, const char*);
    bool (*match_star)(const char*, const char*);

    match_here = (bool (*)(const char*, const char*))[](const char *p, const char *s) {
        for (;; p++, s++) {
            if (!*p) return !*s;
            if (*p == '*') return match_star(p + 1, s);
            if (*p != '?' && *p != *s) return false;
            if (!*s) return false;
        }
    };

    match_star = (bool (*)(const char*, const char*))[](const char *p, const char *s) {
        do {
            if (match_here(p, s)) return true;
        } while (*s++);
        return false;
    };

    return match_here(pat, str) ? 1 : 0;
}