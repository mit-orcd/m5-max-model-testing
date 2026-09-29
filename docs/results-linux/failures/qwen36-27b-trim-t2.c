void trim(char *s)
{
    if (!s)
        return;

    /* Strip leading whitespace */
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    /* If the string is all whitespace or empty */
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    /* Strip trailing whitespace */
    char *end = start;
    while (*end)
        end++;
    end--;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    /* Shift the trimmed substring to the beginning */
    size_t len = (size_t)(end - start + 1);
    memmove(s, start, len);
    s[len] = '\0';
}