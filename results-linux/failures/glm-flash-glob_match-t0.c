#include <stddef.h>

static int match_star(const char *s, const char *p, const char **ss, const char **sp);

int glob_match(const char *pat, const char *str) {
    const char *s = str;
    const char *p = pat;

    while (*s) {
        if (*p == '*') {
            if (match_star(s, p, &s, &p)) {
                return 1;
            }
            return 0;
        } else if (*p == '?' || *p == *s) {
            s++;
            p++;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}

static int match_star(const char *s, const char *p, const char **ss, const char **sp) {
    const char *star = *sp;
    const char *match = *ss;
    const char *tmp;

    while (*match) {
        if (*p == '?' || *p == *match) {
            p++;
            match++;
        } else {
            if (*p != '*') {
                return 0;
            }
            p++;
            tmp = match;
            if (match_star(match, p, &match, &p)) {
                return 1;
            }
            match = tmp + 1;
        }
    }

    if (*p == '*') {
        p++;
    }

    *ss = match;
    *sp = p;
    return 1;
}