void trim(char *s) {
    if (s == NULL) return;

    // Strip leading whitespace
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    // If the string is all whitespace or empty, make it empty
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    // Strip trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    *(end + 1) = '\0';

    // Move the trimmed string to the beginning if needed
    if (start != s) {
        memmove(s, start, (size_t)(end - start + 2));
    }
}