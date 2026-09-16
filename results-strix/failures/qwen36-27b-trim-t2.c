void trim(char *s)
{
    if (s == NULL)
        return;

    /* Strip leading whitespace */
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    /* Strip trailing whitespace */
    char *end = start + strlen(start) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    /* Shift the trimmed string to the beginning */
    size_t len = (size_t)(end - start + 1);
    if (start != s)
        memmove(s, start, len);

    s[len] = '\0';
}