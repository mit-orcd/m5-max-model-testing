int glob_match(const char *pat, const char *str) {
    const char *star = NULL;
    const char *str_match = NULL;

    while (*str) {
        switch (*pat) {
        case '?':
            str++;
            pat++;
            break;
        case '*':
            star = pat++;
            str_match = str;
            break;
        default:
            if (*pat != *str) {
                if (star) {
                    pat = star + 1;
                    str_match++;
                    str = str_match;
                } else {
                    return 0;
                }
            } else {
                str++;
                pat++;
            }
            break;
        }
    }

    while (*pat == '*') {
        pat++;
    }

    return *pat == '\0';
}