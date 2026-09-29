#include <string.h>

void trim(char *s)
{
    if (!s) return;

    /* Skip leading whitespace */
    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n'))
        ++start;

    /* If the string is all whitespace, make it empty */
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    /* Find the last non‑whitespace character */
    char *end = start + strlen(start) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n'))
        --end;

    /* Move the trimmed string to the beginning */
    size_t len = end - start + 1; /* number of characters to keep */
    memmove(s, start, len + 1);   /* +1 for the terminating '\0' */
}