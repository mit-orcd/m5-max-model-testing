void trim(char *s) {
    if (s == NULL) return;

    // Find start of non-whitespace
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    // If string was all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    // Find end of non-whitespace
    char *end = start;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    // Move characters to beginning if necessary
    if (start != s) {
        memmove(s, start, (size_t)(end - start + 1));
    }

    // Null-terminate
    s[end - start + 1] = '\0';
}