int glob_match(const char *pat, const char *str) {
    while (*pat && *str) {
        if (*pat == '*') {
            while (*pat == '*') pat++;
            if (!*pat) return 1;
            while (*str && !glob_match(pat, str)) str++;
            return *str == '\0' ? 1 : 0;
        }
        if (*pat == '?') {
            pat++;
            str++;
            continue;
        }
        if (*pat != *str) return 0;
        pat++;
        str++;
    }
    while (*pat == '*') pat++;
    return *pat == '\0' && *str == '\0';
}