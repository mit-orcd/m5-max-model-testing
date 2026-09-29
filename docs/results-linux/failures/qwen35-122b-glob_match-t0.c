#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *star_pat = NULL;
    const char *star_str = NULL;

    while (*str != '\0') {
        if (*pat == '*') {
            star_pat = pat;
            star_str = str;
            pat++;
        } else if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else if (star_pat != NULL) {
            pat = star_pat + 1;
            str = star_str + 1;
            star_str = str;
        } else {
            return 0;
        }
    }

    while (*pat == '*') {
        pat++;
    }

    return (*pat == '\0');
}