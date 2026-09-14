#include <string.h>

static int glob_match_helper(const char *pat, const char *str) {
    if (*pat == '\0') return *str == '\0';
    if (*pat == '*') {
        /* '*' matches empty sequence, or consume one char from str and retry */
        if (glob_match_helper(pat, str + 1)) return 1;
        return glob_match_helper(pat + 1, str);
    }
    if (*pat == '?') {
        if (*str == '\0') return 0;
        return glob_match_helper(pat + 1, str + 1);
    }
    if (*pat == *str) {
        return glob_match_helper(pat + 1, str + 1);
    }
    return 0;
}

int glob_match(const char *pat, const char *str) {
    return glob_match_helper(pat, str);
}