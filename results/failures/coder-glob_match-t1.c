int glob_match(const char *pat, const char *str) {
    if (!pat || !str) return 0;
    
    const char *star = NULL;
    const char *mark = NULL;
    
    while (*str) {
        if (*pat == '*') {
            star = pat;
            mark = str;
            pat++;
            continue;
        }
        
        if (*pat == '?' || *pat == *str) {
            pat++;
            str++;
            continue;
        }
        
        if (*pat != *str) {
            if (!star) return 0;
            pat = star + 1;
            str = mark + 1;
            mark++;
            continue;
        }
        
        pat++;
        str++;
    }
    
    while (*pat == '*') pat++;
    return !*pat;
}