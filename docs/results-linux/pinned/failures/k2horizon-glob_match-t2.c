int glob_match(const char *pat, const char *str) {
    const char *p = pat, *s = str;
    const char *star = NULL;
    const char *mark = NULL;

    while (*s) {
        if (*p == '*') {
            star = p++;
            mark = s;
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else if (star) {
            p = star + 1;
            s = ++mark;
        } else {
            return 0;
        }
    }

    while (*p == '*') p++;

    return *p == '\0';
}