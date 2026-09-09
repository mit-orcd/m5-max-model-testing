```c
#include <stddef.h>

int glob_match(const char *pat, const char *str) {
    while (*pat) {
        if (*pat == '*') {
            while (*pat == '*')
                pat++;
            if (*pat == '\0')
                return 1;
            for (const char *s = str; ; s++) {
                if (glob_match(pat, s))
                    return 1;
                if (*s == '\0')