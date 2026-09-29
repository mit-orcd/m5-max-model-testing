void trim(char *s) {
    if (s == NULL) return;

    // Find the first non-whitespace character
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    // If the string is all whitespace, make it empty
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    // Find the last non-whitespace character
    char *end = s + strlen(s) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    // Move the trimmed substring to the beginning of the original string
    if (start != s) {
        memmove(s, start, (size_t)(end - start + 1));
    }

    // Null-terminate the string
    *(s + (end - start + 1)) = '\0';
}