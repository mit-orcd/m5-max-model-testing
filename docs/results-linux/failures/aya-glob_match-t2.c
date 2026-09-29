#include <string.h>

int glob_match(const char *pat, const char *str) {
    int p = 0, s = 0;
    while (1) {
        if (pat[p] == '*') {
            if (str[s] == '\0')
                return 1;
            p++;
        } else if (pat[p] == '?') {
            if (str[s] == '\0' || ++s == strlen(str))
                return 0;
        } else if (pat[p] == str[s]) {
            p++; s++;
        } else {
            if (str[s] == '\0' || memcmp(pat + p, str + s, strlen(str) - s) != 0)
                return 0;
            p = strlen(pat);
        }
    }
}