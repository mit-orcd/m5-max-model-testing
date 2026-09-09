void trim(char *s) {
    char *start = s;
    char *end;

    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    *(end + 1) = '\0';

    if (start != s) {
        memmove(s, start, end - start + 2);
    }
}