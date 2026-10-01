void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start != '\0' && (*start == ' ' || *start == '\t' || *start == '\n')) {
        start++;
    }

    char *end = s;
    while (*end != '\0') {
        end++;
    }
    end--;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    char *dest = s;
    while (start <= end) {
        *dest++ = *start++;
    }
    *dest = '\0';
}