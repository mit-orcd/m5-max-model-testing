#include <string.h>

void trim(char *s)
{
    char *p;
    char *end;

    if (s == NULL)
        return;

    p = s;
    while (*p == ' ' || *p == '\t' || *p == '\n')
        ++p;

    end = p + strlen(p);
    while (end > p && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n'))
        --end;

    memmove(s, p, (size_t)(end - p) + 1);
}