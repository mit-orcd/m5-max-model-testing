#include <ctype.h>

void trim(char *s)
{
    if (s == NULL)
        return;

    char *start = s;
    while (*start && isspace((unsigned char)*start))
        start++;

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end))
        end--;

    *(end + 1) = '\0';

    if (start != s)
        memmove(s, start, (size_t)(end - start + 2));
}