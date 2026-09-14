#include <stddef.h>

void trim(char *s)
{
    if (s == NULL)
        return;

    /* Strip leading whitespace */
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    /* If the string was all whitespace or empty, just null-terminate */
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    /* Strip trailing whitespace */
    char *end = start;
    while (*end != '\0')
        end++;
    end--; /* Back up from the null terminator */
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    /* Shift the trimmed substring to the beginning */
    size_t len = (size_t)(end - start + 1);
    if (start != s) {
        /* Use memmove to handle overlapping regions safely */
        memmove(s, start, len);
    }
    s[len] = '\0';
}