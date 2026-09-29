#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    while (*pat && *str) {
        if (*pat == '*') {
            while (*pat == '*') pat++;
            if (!*pat) return 1;
            do {
                if (glob_match(pat, str)) return 1;
            } while (*str++);
            return 0;
        } else if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else {
            return 0;
        }
    }
    return !*pat && !*str;
}