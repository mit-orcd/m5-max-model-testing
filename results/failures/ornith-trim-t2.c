void trim(char *s) {
    char *end;

    /* Skip leading whitespace */
    while (*s == ' ' || *s == '\t' || *s == '\n')
        s++;

    /* Remove the whole string if it was all whitespace */
    if (*s == '\0') {
        *s = '\0';
        return;
    }

    /* Point to the last character */
    end = s + strlen(s) - 1;

    /* Trim trailing whitespace */
    while (end > s && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    /* Terminate the string after the trailing whitespace */
    *(end + 1) = '\0';
}