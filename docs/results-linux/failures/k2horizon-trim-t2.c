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

    end = start + 1;
    while (*end != '\0') {
        end++;
    }

    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) {
        end--;
    }

    *end = '\0';

    while (start > s) {
        *--end = *start;
        start++;
    }
}