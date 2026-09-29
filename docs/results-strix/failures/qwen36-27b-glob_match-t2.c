int glob_match(const char *pat, const char *str) {
    const char *p = pat, *s = str;
    const char *star_pat = NULL, *star_str = NULL;

    while (*s) {
        if (*p == '?' || (*p != '*' && *p == *s)) {
            p++;
            s++;
        } else if (*p == '*') {
            star_pat = p;
            star_str = s;
            p++;
        } else if (star_pat) {
            p = star_pat + 1;
            star_str++;
            s = star_str;
        } else {
            return 0;
        }
    }

    while (*p == '*') {
        p++;
    }

    return *p == '\0';
}