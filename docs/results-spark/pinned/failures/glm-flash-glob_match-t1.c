#include <stddef.h>

static int match_star(const char *pat, const char *str, const char **next_pat);
static int match_question(const char *pat, const char *str);

int glob_match(const char *pat, const char *str) {
    if (*pat == '*') {
        return match_star(pat, str, &pat);
    }
    return match_question(pat, str);
}

static int match_star(const char *pat, const char *str, const char **next_pat) {
    const char *s = str;
    const char *p = pat;
    const char *star = NULL;
    const char *match = NULL;

    while (*s != '\0') {
        if (*p == '*') {
            star = p;
            match = s;
            p++;
        } else if (*p == *s || *p == '?') {
            p++;
            s++;
        } else {
            if (star != NULL) {
                p = star + 1;
                match++;
                s = match;
            } else {
                return 0;
            }
        }
    }

    while (*p == '*') {
        p++;
    }

    if (*p == '\0') {
        *next_pat = p;
        return 1;
    }

    return 0;
}

static int match_question(const char *pat, const char *str) {
    if (*pat == '\0' && *str == '\0') {
        return 1;
    }
    if (*pat == '?' && *str != '\0') {
        return match_question(pat + 1, str + 1);
    }
    if (*pat == *str) {
        return match_question(pat + 1, str + 1);
    }
    return 0;
}