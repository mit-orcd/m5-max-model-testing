int glob_match(const char *pat, const char *str) {
    const char *s = str;
    const char *p = pat;
    const char *star = NULL;
    const char *match = NULL;

    while (*s) {
        if (*p == '?') {
            s++;
            p++;
        } else if (*p == '*') {
            star = p++;
            match = s;
        } else if (*p == *s) {
            s++;
            p++;
        } else {
            if (star) {
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

    return *p == '\0';
}