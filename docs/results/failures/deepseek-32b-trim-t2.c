void trim(char *s) {
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t' || s[start] == '\n') {
        start++;
    }
    if (start >= strlen(s)) {
        s[0] = '\0';
        return;
    }
    int end = strlen(s) - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\n')) {
        end--;
    }
    int len = end - start + 1;
    for (int i = 0; i < len; i++) {
        s[i] = s[start + i];
    }
    s[len] = '\0';
}