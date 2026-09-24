void trim(char *s) {
    if (!s) return;
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') start++;
    char *end = start;
    while (*end) end++;
    if (end == start) {
        *start = '\0';
        return;
    }
    end--;
    while (*end == ' ' || *end == '\t' || *end == '\n') end--;
    *(end + 1) = '\0';
    if (start != s) {
        char *dst = s;
        while (start <= end) {
            *dst++ = *start++;
        }
        *dst = '\0';
    }
}