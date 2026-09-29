int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *str_mark = NULL;

    while (*str) {
        switch (*pat) {
        case '?':
            str++;
            pat++;
            break;
        case '*':
            star = pat++;
            str_mark = str;
            break;
        default:
            if (*pat == *str) {
                str++;
                pat++;
            } else if (star) {
                pat = star + 1;
                str_mark++;
                str = str_mark;
            } else {
                return 0;
            }
        }
    }
    while (*pat == '*') {
        pat++;
    }
    return !*pat;
}