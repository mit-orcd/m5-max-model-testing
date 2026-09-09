#include <string.h>

int glob_match(const char *pat, const char *str) {
    int p = 0, s = 0;
    while (1) {
        if (pat[p] == '*') {
            if (str[s] == '\0')
                return 1;
            p++;
        } else if (pat[p] == '?' && str[s] != '\0') {
            p++;
            s++;
        } else if (pat[p] == str[s]) {
            p++;
            s++;
        } else {
            return 0;
        }
    }
}