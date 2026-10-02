#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    while (*str != '\0') {
        if (*pat == '\0' || (*pat != '*' && *pat != '?' && *pat != *str)) {
            if (*pat == '*') {
                if (*(pat + 1) == '\0') return 1;
                for (const char *tmp = str; *tmp != '\0'; ++tmp) {
                    if (glob_match(pat + 1, tmp)) return 1;
                }
                return 0;
            }
            if (*pat == '?' || *pat == *str) {
                ++pat;
                ++str;
            } else {
                return 0;
            }
        } else {
            return 0;
        }
    }
    while (*pat == '*') ++pat;
    return *pat == '\0';
}