#include <ctype.h>

void trim(char *s)
{
    if (s == NULL)
        return;

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = start + __builtin_strlen(start);
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    end[1] = '\0';

    if (start != s)
        for (char *i = start; *i; i++)
            s[i - start] = *i;
}