void trim(char *s) {
    char *end;

    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    *(end + 1) = '\0';
}