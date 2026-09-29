#include <stddef.h>

int glob_match(const char *pat, const char *str) {
    while (*pat) {
        if (*pat == '*') {
            if (!*str) return 1;
            for (size_t i = 0; str[i]; i++) {
                if (glob_match(pat + 1, str + i)) return 1;
            }
            return 0;
        } else if (*pat == '?') {
            if (!*str) return 0;
            pat++;
            str++;
        } else {
            if (*pat != *str) return 0;
            pat++;
            str++;
        }
    }
    return !*str;
}