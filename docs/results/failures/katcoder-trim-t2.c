void trim(char *s) {
    char *end;
    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;
    if (*s == '\0') {
        *s = '\0';
        return;
    }
    end = s;
    while (*end != '\0')
        end++;
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n'))
        end--;
    *end = '\0';
}