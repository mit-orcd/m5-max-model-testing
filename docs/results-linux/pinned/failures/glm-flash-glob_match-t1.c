#include <stddef.h>
#include <stdbool.h>

static bool match_star(const char *s, const char *p, const char **next_s, const char **next_p);

int glob_match(const char *pat, const char *str) {
    if (*pat == '*') {
        return match_star(str, pat, &str, &pat);
    }
    for (; *str && *pat; str++, pat++) {
        if (*pat == '*') {
            return match_star(str, pat, &str, &pat);
        }
        if (*pat == '?' || *pat == *str) {
            continue;
        }
        return 0;
    }
    return (*pat == '\0' && *str == '\0');
}

static bool match_star(const char *s, const char *p, const char **next_s, const char **next_p) {
    const char *star = *p;
    const char *match = *next_s;
    const char *tmp_s = *next_s;
    const char *tmp_p = *next_p;

    for (;;) {
        if (*tmp_p == '\0') {
            *next_s = tmp_s;
            *next_p = tmp_p;
            return true;
        }
        if (*tmp_p == '*') {
            tmp_p++;
            continue;
        }
        if (*tmp_p == '?' || *tmp_p == *tmp_s) {
            tmp_p++;
            tmp_s++;
            continue;
        }
        if (match != *next_s) {
            tmp_s = ++match;
            continue;
        }
        return false;
    }
}