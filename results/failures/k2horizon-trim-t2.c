void trim(char *s) {
    char *end;
    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;
    if (*s == '\0') {
        *s = '\0';
        return;
    }
    end = s + strlen(s) - 1;
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;
    end[1] = '\0';
}