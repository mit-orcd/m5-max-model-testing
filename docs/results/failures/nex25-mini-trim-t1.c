void trim(char *s) {
    char *end;

    while (*s == ' ' || *s == '\t' || *s == '\n') {
        ++s;
    }

    if (*s == '\0') {
        return;
    }

    end = s + 1;
    while (*end != '\0') {
        ++end;
    }

    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n')) {
        --end;
    }

    *(++end) = '\0';
}