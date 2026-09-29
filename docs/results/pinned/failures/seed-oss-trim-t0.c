void trim(char *s) {
    if (!s) return;

    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n'))
        start++;

    if (!*start) {
        *s = '\0';
        return;
    }

    char *end = start;
    while (*(end + 1))
        end++;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    size_t len = end - start + 1;
    for (size_t i = 0; i < len; i++)
        s[i] = start[i];
    s[len] = '\0';
}