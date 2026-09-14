void trim(char *s) {
    if (!s) return;
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') start++;
    if (start == s + strlen(s)) {
        s[0] = '\0';
        return;
    }
    char *end = s + strlen(s) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    size_t len = end - start + 1;
    memmove(s, start, len);
    s[len] = '\0';
}