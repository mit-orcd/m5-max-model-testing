void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace(s[start])) start++;
    if (start >= len) {
        *s = '\0';
        return;
    }
    size_t end = len - 1;
    while (end >= start && isspace(s[end])) end--;
    size_t new_len = end - start + 1;
    for (size_t i = 0; i < new_len; i++) s[i] = s[start + i];
    s[new_len] = '\0';
}