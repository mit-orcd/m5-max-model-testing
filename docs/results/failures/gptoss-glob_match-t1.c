int glob_match(const char *pat, const char *str)
{
    const char *p = pat;
    const char *s = str;
    const char *star = NULL;
    const char *ss = NULL;

    while (*s) {
        if (*p == '?' || *p == *s) {
            ++p;
            ++s;
        } else if (*p == '*') {
            star = p++;
            ss = s;
        } else if (star) {
            p = star + 1;
            s = ++ss;
        } else {
            return 0;
        }
    }

    while (*p == '*')
        ++p;

    return *p == '\0' ? 1 : 0;
}