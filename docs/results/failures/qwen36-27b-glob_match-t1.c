#include <stddef.h>

static int glob_match_tail(const char *pat, const char *str) {
    while (*str != '\0' && *pat != '*') {
        if (*pat != '?' && *pat != *str) {
            return 0;
        }
        pat++;
        str++;
    }
    while (*pat == '*') {
        pat++;
    }
    return (*pat == '\0') ? 1 : 0;
}

int glob_match(const char *pat, const char *str) {
    while (*pat != '\0') {
        if (*pat == '*') {
            if (*(pat + 1) == '\0') {
                return 1;
            }
            do {
                if (glob_match_tail(pat + 1, str)) {
                    return 1;
                }
                str++;
            } while (*str != '\0');
            return 0;
        } else if (*pat != '?' && *pat != *str) {
            return 0;
        } else {
            pat++;
            str++;
        }
    }
    return (*str == '\0') ? 1 : 0;
}