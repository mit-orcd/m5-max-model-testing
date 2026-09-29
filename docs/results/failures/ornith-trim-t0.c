void trim(char *s) {
    char *end;

    /* Skip leading whitespace */
    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    /* Find end of string */
    end = s + strlen(s);

    /* Move back over trailing whitespace */
    if (end != s) {
        end--;
        while (end > s && (*end == ' ' || *end == '\t' || *end == '\n'))
            end--;
        end++;
        *end = '\0';
    }
}