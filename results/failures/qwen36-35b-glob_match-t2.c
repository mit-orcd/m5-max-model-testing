int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *str_tmp = NULL;

    while (*str) {
        if (*pat == *str || *pat == '?') {
            pat++;
            str++;
        } else if (*pat == '*') {
            star = pat++;
            str_tmp = str;
        } else if (star) {
            pat = star + 1;
            str = ++str_tmp;
        } else {
            return 0;
        }
    }
    while (*pat == '*') {
        pat++;
    }
    return *pat == '\0';
}