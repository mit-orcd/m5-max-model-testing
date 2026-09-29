int glob_match(const char *pat, const char *str) {
    int p = 0, s = 0;
    while (1) {
        if (pat[p] == '*') {
            if (str[s] == '\0') return 1;
            p++;
        } else if (pat[p] == '?') {
            if (str[s] == '\0' || str[s] != pat[p + 1]) return 0;
            p++; s++;
        } else {
            if (str[s] != pat[p]) return 0;
            p++; s++;
        }
    }
}