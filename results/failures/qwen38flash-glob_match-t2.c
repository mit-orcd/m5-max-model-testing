int glob_match(const char *pat, const char *str) {
    const char *p = pat;
    const char *s = str;
    const char *star_p = NULL;
    const char *star_s = NULL;

    while (*s) {
        if (*p == '*') {
            star_p = p;
            star_s = s;
            p++;
        } else if (*p == '?' || *p == *s) {
            p++;
            s++;
        } else if (star_p) {
            p = star_p + 1;
            star_s++;
            s = star_s;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0' ? 1 : 0;
}