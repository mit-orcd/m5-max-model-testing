#include <string.h>
#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    const char *p = NULL, *s = NULL, *match = NULL, *star = NULL;
    bool found = false;

    for (p = pat, s = str; *s; ++s) {
        if (*p == '?') {
            ++p;
        } else if (*p == '*') {
            star = p++;
            match = s;
            found = true;
        } else if (*p != *s) {
            if (!found) return 0;
            p = star + 1;
            s = match++;
        } else {
            ++p;
        }
    }

    while (*p == '*') ++p;
    return !*p;
}