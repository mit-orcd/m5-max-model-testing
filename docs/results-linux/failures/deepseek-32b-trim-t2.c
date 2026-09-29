void trim(char *s) {
    if (!s || *s == '\0') return;

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    char *end = start;
    while (*end != '\0') {
        end++;
    }

    end--;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    if (start > s) {
        memmove(s, start, end - start + 1);
    }

    *(end + 1) = '\0';
}