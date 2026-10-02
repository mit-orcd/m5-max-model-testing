void trim(char *s) {
    if (!s) return;

    // Strip leading whitespace
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    // If the string is all whitespace or empty, null-terminate immediately
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    // Strip trailing whitespace
    char *end = s + strlen(s) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    *(end + 1) = '\0';

    // Move the trimmed string to the beginning if necessary
    if (start != s) {
        memmove(s, start, (size_t)(end - start + 1));
    }
}