#include <stdbool.h>

int glob_match(const char *pat, const char *str) {
    bool match[2][256];
    match[0][0] = true;

    for (int i = 1; i <= 256; i++) {
        match[0][i] = false;
    }

    while (*pat) {
        bool next[2][256];
        for (int i = 0; i <= 256; i++) {
            next[0][i] = false;
            next[1][i] = false;
        }

        if (*pat == '*') {
            for (int i = 0; i <= 256; i++) {
                next[0][i] = match[0][i] || match[1][i];
            }
            pat++;
        } else if (*pat == '?') {
            for (int i = 0; i < 256; i++) {
                next[1][i + 1] = match[0][i];
            }
            pat++;
        } else {
            for (int i = 0; i < 256; i++) {
                if (match[0][i] && str[i] == *pat) {
                    next[1][i + 1] = true;
                }
            }
            pat++;
        }

        for (int i = 0; i <= 256; i++) {
            match[0][i] = next[0][i];
            match[1][i] = next[1][i];
        }
    }

    return match[0][256 - 1];
}