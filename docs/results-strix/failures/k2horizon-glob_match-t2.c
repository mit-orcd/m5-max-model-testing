#include <stddef.h>

static int match_here(const char *pat, const char *str) {
    while (*pat == '*') {
        pat++;
        if (*pat == '\0') return 1;
    }

    if (*pat == '\0') return *str == '\0';

    if (*pat == '?') {
        if (*str == '\0') return 0;
        return match_here(pat + 1, str + 1);
    }

    if (*pat != *str) return 0;

    return match_here(pat + 1, str + 1);
}

int glob_match(const char *pat, const char *str) {
    return match_here(pat, str);
}