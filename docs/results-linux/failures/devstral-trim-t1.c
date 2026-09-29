void trim(char *s) {
    char *end;
    if (*s == '\0') return;
    while (*s != '\0' && (*s == ' ' || *s == '\t' || *s == '\n')) s++;
    end = s + strlen(s) - 1;
    while (end >= s && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    *(end + 1) = '\0';
}