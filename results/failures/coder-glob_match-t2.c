int glob_match(const char *pat, const char *str) {
    if (!pat || !str) return 0;
    
    const char *star = NULL;
    const char *ss = str;
    const char *ps = pat;
    
    while (*str) {
        if (*ps == '*') {
            star = ps;
            ps++;
        } else if (*ps == '?') {
            ps++;
            str++;
        } else if (*pat && *ps == *str) {
            ps++;
            str++;
        } else if (star) {
            ps = star + 1;
            str = ++ss;
        } else {
            return 0;
        }
    }
    
    while (*ps == '*') ps++;
    return *ps == '\0';
}