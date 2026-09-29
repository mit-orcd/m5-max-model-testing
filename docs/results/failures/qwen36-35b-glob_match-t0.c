int glob_match(const char *pat, const char *str) {
    const char *s = str;
    const char *p = pat;
    const char *star = NULL;
    const char *match = NULL;

    while (*s) {
        switch (*p) {
        case '?':
            s++;
            p++;
            break;
        case '*':
            star = p++;
            match = s;
            break;
        default:
            if (*p == *s) {
                s++;
                p++;
            } else {
                if (star) {
                    p = star + 1;
                    s = ++match;
                } else {
                    return 0;
                }
            }
            break;
        }
    }
    while (*p == '*') {
        p++;
    }
    return *p == '\0';
}