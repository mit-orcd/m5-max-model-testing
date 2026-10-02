void trim(char *s) {
    if (!s) return;

    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n'))
        start++;

    char *end = s;
    while (*end) end++;
    end--;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    if (start > end) {
        *s = '\0';
    } else {
        char *dst = s;
        while (dst <= end)
            *dst++ = *start++;
        *dst = '\0';
    }
}