void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start != '\0' && (*start == ' ' || *start == '\t' || *start == '\n')) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = start;
    while (*end != '\0') {
        end++;
    }
    end--;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    char *dst = s;
    while (dst <= end) {
        *dst++ = *start++;
    }
    *dst = '\0';
}