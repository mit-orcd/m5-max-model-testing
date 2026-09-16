int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *cp = NULL;
    while (*str) {
        if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else if (*pat == '*') {
            star = pat++;
            cp = str;
        } else if (star) {
            pat = star + 1;
            str = ++cp;
        } else {
            return 0;
        }
    }
    while (*pat == '*') pat++;
    return !*pat;
}