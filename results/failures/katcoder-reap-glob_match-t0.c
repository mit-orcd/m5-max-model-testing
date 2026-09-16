int glob_match(const char *pat, const char *str) {
    int plen = 0, slen = 0;
    for (const char *p = pat; *p; p++) plen++;
    for (const char *s = str; *s; s++) slen++;
    return glob_match_rec(pat, plen, str, slen);
}

int glob_match_rec(const char *pat, int plen, const char *str, int slen) {
    if (plen == 0) return slen == 0;
    if (pat[0] == '*') {
        for (int i = 0; i <= slen; i++) {
            if (glob_match_rec(pat + 1, plen - 1, str + i, slen - i)) return 1;
        }
        return 0;
    }
    if (pat[0] == '?' && slen > 0) {
        return glob_match_rec(pat + 1, plen - 1, str + 1, slen - 1);
    }
    if (slen > 0 && pat[0] == str[0]) {
        return glob_match_rec(pat + 1, plen - 1, str + 1, slen - 1);
    }
    return 0;
}