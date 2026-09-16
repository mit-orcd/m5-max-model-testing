#include <ctype.h>

int glob_match(const char *pat, const char *str) {
    while (*pat) {
        if (*pat == '*') {
            while (*pat == '*') pat++;
            if (!*pat) return 1;
            while (*str) {
                if (glob_match(pat, str)) return 1;
                str++;
            }
            return 0;
        }
        if (*str == '\0') return 0;
        if (*pat == '?') {
            pat++;
            str++;
        } else if (pat[0] == '\\' && (pat[1] == '*' || pat[1] == '?' || pat[1] == '\\')) {
            pat += 2;
            str++;
        } else {
            if (!tolower((unsigned char)*pat) == tolower((unsigned char)*str)) return 0;
            pat++;
            str++;
        }
    }
    return *str == '\0';
}