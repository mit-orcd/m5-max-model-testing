int glob_match(const char *pat, const char *str)
{
    const char *p = pat, *s = str;
    const char *star = NULL, *ss = NULL;

    while (*s) {
        if (*p == *s || *p == '?') {
            ++p;
            ++s;
        } else if (*p == '*') {
            star = p;
            ss = s;
            ++p;
        } else if (star) {
            p = star + 1;
            ++ss;
            s = ss;
        } else {
            return 0;
        }
    }

    while (*p == '*')
        ++p;

    return *p == '\0';
}