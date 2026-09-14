#include <stddef.h>

static int glob_match_core(const char *pat, const char *str) {
    while (*pat && *str) {
        if (*pat == '*') {
            while (*pat == '*') pat++;
            if (*pat == '\0') return 1;
            while (*str) {
                if (glob_match_core(pat, str)) return 1;
                str++;
            }
            return 0;
        } else if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else {
            return 0;
        }
    }
    return *pat == '\0' && *str == '\0';
}

int glob_match(const char *pat, const char *str) {
    if (pat == NULL || str == NULL) return 0;
    return glob_match_core(pat, str);
}