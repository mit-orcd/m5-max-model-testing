```c
#include <stddef.h>
#include <stdlib.h>

int glob_match(const char *pat, const char *str)
{
    if (!pat || !str) {
        return !pat && !str;
    }

    const size_t len = strlen(str);
    unsigned char *match = calloc(len + 1, sizeof(*match));
    if (!match) {
        return 0;
    }

    match[0] = 1;

    for (size_t i = 0; pat[i] != '\0'; ++i) {
        if (pat[i] == '*') {
            match[0] = 1;
            for (size_t j = 1; j <= len; ++j) {
                match[j] = match[j] || match[j - 1];
            }
        } else {
            for (size_t j = len; j > 0; --j) {
                match[j] = (str[j - 1] == pat[i]) && match[j - 1];
            }
            match[0] = 0;
        }
    }