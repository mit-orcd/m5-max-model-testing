int glob_match(const char *pat, const char *str) {
    const char *patCur = pat;
    const char *strCur = str;
    const char *starPat = NULL;
    const char *starStr = NULL;

    while (*strCur) {
        switch (*patCur) {
        case '?':
            strCur++;
            patCur++;
            break;
        case '*':
            starPat = patCur++;
            starStr = strCur;
            break;
        default:
            if (*patCur != *strCur) {
                if (starPat) {
                    patCur = starPat + 1;
                    strCur = ++starStr;
                } else {
                    return 0;
                }
            } else {
                strCur++;
                patCur++;
            }
            break;
        }
    }
    while (*patCur == '*') {
        patCur++;
    }
    return *patCur == '\0';
}