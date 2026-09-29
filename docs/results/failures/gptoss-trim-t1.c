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

    /* Find the end of the string */
    char *end = start;
    while (*end != '\0')
        ++end;

    /* Skip trailing whitespace */
    while (end > start && (*(end - 1) == ' ' || *(end - 1) == '\t' || *(end - 1) == '\n'))
        --end;

    /* Move the trimmed string to the beginning */
    size_t len = end - start;          /* length of the trimmed string */
    if (start != s)                    /* shift only if needed */
        memmove(s, start, len + 1);    /* +1 to copy the terminating '\0' */
}