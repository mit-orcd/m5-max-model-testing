int glob_match(const char *pat, const char *str) {
    if (!pat || !str) return 0;
    
    const char *star = NULL;
    const char *str_star = NULL;
    
    while (*str) {
        if (*pat == '*') {
            star = pat;
            str_star = str;
            pat++;
        } else if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
        } else if (star) {
            pat = star + 1;
            str = str_star + 1;
            str_star++;
        } else {
            return 0;
        }
    }
    
    while (*pat == '*') pat++;
    return !*pat;
}