int glob_match(const char *pat, const char *str)
{
    const char *star = NULL;
    const char *match = NULL;

    if (pat == NULL || str == NULL)
        return 0;

    while (*str != '\0') {
        if (*pat == '?') {
            ++pat;
            ++str;
        } else if (*pat == '*') {
            star = pat++;
            match = str;
        } else if (*pat != '\0' && *pat == *str) {
            ++pat;
            ++str;
        } else if (star != NULL) {
            pat = star + 1;
            str = ++match;
        } else {
            return 0;
        }
    }

    while (*pat == '*')
        ++pat;

    return *pat == '\0';
}