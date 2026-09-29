int glob_match(const char *pat, const char *str) {
    if (!pat) return !str;
    if (!str) return !*pat;

    const char *star = NULL;
    const char *str_match = str;

    while (*str) {
        if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else if (*pat == '*') {
            star = pat++;
            str_match = str;
        } else if (star) {
            pat = star + 1;
            str_match++;
            str = str_match;
        } else {
            return 0;
        }
    }

    while (*pat == '*') {
        pat++;
    }

    return !*pat;
}