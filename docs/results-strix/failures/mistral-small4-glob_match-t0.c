#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    while (*pat != '\0') {
        switch (*pat) {
            case '?':
                if (*str == '\0') return 0;
                str++;
                pat++;
                break;
            case '*':
                if (*str == '\0') return 1;
                if (*(pat + 1) == '\0') return 1;
                if (*pat == '*' && *(pat + 1) == '*') {
                    pat++;
                    break;
                }
                if (*str == *pat) {
                    str++;
                    pat++;
                } else {
                    return glob_match(pat + 1, str) || glob_match(pat, str + 1);
                }
                break;
            default:
                if (*str != *pat) return 0;
                str++;
                pat++;
                break;
        }
    }
    return *str == '\0';
}