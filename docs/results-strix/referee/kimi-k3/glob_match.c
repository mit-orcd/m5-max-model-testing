int glob_match(const char *pat, const char *str) {
    const char *star = 0, *ss = 0;
    while (*str) {
        if (*pat == '?' || *pat == *str) { pat++; str++; }
        else if (*pat == '*') { star = pat++; ss = str; }
        else if (star) { pat = star + 1; str = ++ss; }
        else return 0;
    }
    while (*pat == '*') pat++;
    return *pat == 0;
}
