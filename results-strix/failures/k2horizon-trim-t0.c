void trim(char *s) {
    char *end;
    while (*s == ' ' || *s == '\t' || *s == '\n') {
        s++;
    }
    end = s + strlen(s);
    while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) {
        end--;
    }
    *end = '\0';
    memmove(s, s + (s - s), end - s);
}