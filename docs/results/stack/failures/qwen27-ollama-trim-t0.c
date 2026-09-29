#include <string.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = s + strlen(s) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    end[1] = '\0';

    if (start != s)
        memmove(s, start, (size_t)(end - start + 1));
}