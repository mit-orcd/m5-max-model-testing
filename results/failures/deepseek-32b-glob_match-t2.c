```c
int glob_match(const char *pat, const char *str) {
    if (*pat == '\0') {
        return *str == '\0';
    }
    if (*pat == '*') {
        if (glob_match(pat + 1, str)) {
            return 1;
        }
        if (*str != '\0' && glob_match(pat, str + 1)) {
            return 1;
        }
        return 0;
    } else if (*pat == '?') {
        if (*str == '\0') {
            return