void trim(char *s) {
    if (s == NULL || *s == '\0') return;

    size_t start = 0;
    while (s[start] == ' ' || s[start] == '\t' || s[start] == '\n') {
        start++;
        if (s[start] == '\0') break;
    }

    if (s[start] == '\0') {
        s[0] = '\0';
        return;
    }

    size_t end = strlen(s) - 1;
    while (end >= start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\n')) {
        end--;
    }

    size_t new_len = end - start + 1;
    memmove(s, s + start, new_len);
    s[new_len] = '\0';
}