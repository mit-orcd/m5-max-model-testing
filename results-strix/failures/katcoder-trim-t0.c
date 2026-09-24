void trim(char *s) {
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;
    char *end = start;
    while (*end)
        end++;
    end--;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;
    char *d = start;
    while (start <= end)
        *d++ = *start++;
    *d = '\0';
}