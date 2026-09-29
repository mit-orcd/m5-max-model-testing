int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *str_match = NULL;

    while (*str) {
        if (*pat == '?' || *pat == *str) {
            str++;
            pat++;
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

    return *pat == '\0';
}