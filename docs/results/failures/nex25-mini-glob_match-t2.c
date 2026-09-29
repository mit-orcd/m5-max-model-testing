#include <stddef.h>

int glob_match(const char *pat, const char *str) {
    size_t pi = 0, si = 0;
    size_t star = SIZE_MAX;
    size_t match = 0;

    while (str[si] != '\0') {
        if (pat[pi] == '?' || pat[pi] == str[si]) {
            ++pi;
            ++si;
        } else if (pat[pi] == '*') {
            star = pi++;
            match = si++;
        } else {
            if (star == SIZE_MAX) {
                return 0;
            }
            pi = star + 1;
            match++;
            si = match;
        }
    }

    while (pat[pi] == '*') {
        ++pi;
    }

    return pat[pi] == '\0';
}