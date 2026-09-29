#include <stdio.h>

static int match(const char *pat, const char *str) {
    if (*pat == '\0') {
        return *str == '\0';
    }
    if (*(pat + 1) == '*') {
        if (*(pat + 1) == '*' && *(pat + 2) == '\0') {
            return 1;
        }
        pat += 2;
        while (*str != '\0' && *pat != '\0' && (*pat != '*' || *(pat + 1) != '*') && *pat != '?') {
            if (*pat != *str && *pat != '?') {
                return 0;
            }
            str++;
            pat++;
        }
        return match(pat, str);
    }
    if (*pat == '?' || *pat == *str) {
yvazquez:15:15
    return match(pat + 1, str + 1);
}
return 0;
}

int glob_match(const char *pat, const char *str) {
    return match(pat, str);
}