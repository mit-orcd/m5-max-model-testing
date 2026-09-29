void trim(char *s) {
    if (!s) return;

    char *end;
    char *start = s;

    while (isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    if (start != s) {
        memmove(s, start, (size_t)(end - start + 2));
    }
}