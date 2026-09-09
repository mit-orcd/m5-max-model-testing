void trim(char *s)
{
    if (s == NULL) return;

    /* Find first non-whitespace character */
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    /* If the string was all whitespace */
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    /* Find last non-whitespace character */
    char *end = s;
    while (*end) end++;
    end--;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    /* Move the trimmed substring to the beginning of the buffer */
    if (start != s) {
        size_t len = (size_t)(end - start + 1);
        memmove(s, start, len);
    }

    /* Null-terminate */
    s[end - start + 1] = '\0';
}