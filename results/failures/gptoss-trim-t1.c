#include <string.h>

void trim(char *s)
{
    if (!s) return;

    /* Find first non‑whitespace character */
    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n'))
        start++;

    /* If string is all whitespace, set to empty string */
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    /* Find last non‑whitespace character */
    char *end = start + strlen(start) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    /* Calculate new length (including null terminator) */
    size_t newlen = end - start + 1;

    /* Move trimmed part to the beginning if needed */
    if (start != s)
        memmove(s, start, newlen);
}