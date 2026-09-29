void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end;

    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = start + __builtin_strlen(start) - 1;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) {
        end--;
    }

    *(end + 1) = '\0';

    if (start != s) {
        __builtin_memmove(s, start, (size_t)(end - start + 2));
    }
}