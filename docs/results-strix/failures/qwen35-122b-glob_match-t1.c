#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *star_pat = NULL;
    const char *star_str = NULL;

    while (*str) {
        if (*pat == '*') {
            star_pat = pat;
            star_str = str;
            pat++;
            // Skip multiple consecutive '*'
            while (*pat == '*') pat++;
            if (!*pat) return 1;
        } else if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else if (star_pat != NULL) {
            // Backtrack: advance the string pointer after the last '*' match
            pat = star_pat + 1;
            str = ++star_str;
        } else {
            return 0;
        }
    }

    // Consume remaining '*' in pattern
    while (*pat == '*') pat++;

    return *pat == '\0';
}