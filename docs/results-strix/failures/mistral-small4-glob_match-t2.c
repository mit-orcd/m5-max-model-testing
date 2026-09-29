#include <stdbool.h>
#include <ctype.h>

static bool match(const char *pat, const char *str) {
    while (*str != '\0') {
        if (*pat == '\0' || (*pat != '*' && *pat != '?' && *pat != *str && !(tolower(*pat) == tolower(*str)))) {
            return false;
        }
        if (*pat == '*' || *pat == '?') {
            if (*pat == '*') {
                while (*pat == '*') {
                    pat++;
                }
                if (*pat == '\0') {
                    return true;
                }
                while (*str != '\0') {
                    if (match(pat, str)) {
                        return true;
                    }
                    str++;
                }
                return false;
            } else {
                pat++;
                str++;
            }
        } else {
            pat++;
            str++;
        }
    }
    while (*pat == '*') {
        pat++;
    }
    return *pat == '\0';
}

int glob_match(const char *pat, const char *str) {
    if (pat == NULL || str == NULL) {
        return 0;
    }
    return match(pat, str) ? 1 : 0;
}