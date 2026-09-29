#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *ss = NULL;

    while (*str) {
        if (*pat == '*') {
            star = pat++;
            ss = str;
            continue;
        }
        if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
            continue;
        }
        if (star) {
            pat = star + 1;
            str = ++ss;
            continue;
        }
        return 0;
    }

    while (*pat == '*') pat++;

    return !*pat;
}