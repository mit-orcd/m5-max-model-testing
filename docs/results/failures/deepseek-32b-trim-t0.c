void trim(char *s) {
    if (!s) return;
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t' || s[start] == '\n') {
        start++;
    }
    int end = strlen(s) - 1;
    while (end >= 0 && (s[end] == ' ' || s[end] == '\t' || s[end] == '\n')) {
        end--;
    }
    if (start > end) {
        s[0] = '\0';
        return;
    }
    int new_len = end - start + 1;
    for (int i = 0; i < new_len; i++) {
        s[i] = s[start + i];
    }
    s[new_len] = '\0';
}