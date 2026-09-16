int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *match = NULL;

    while (*str) {
        if (*pat == *str || (*pat == '?' && *str != '\0')) {
            pat++;
            str++;
        } else if (*pat == '*') {
            star = pat++;
            match = str;
        } else if (star) {
            pat = star + 1;
            str = ++match;
        } else {
            return 0;
        }
    }

    while (*pat == '*') pat++;

    return *pat == '\0';
}