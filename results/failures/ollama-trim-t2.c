```c
#include <string.h>

void trim(char *s) {
    char *start = s;
    char *end;

    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = s + strlen(s) - 1;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;

    size_t len = (size_t)(end - start + 1);
    memmove(s, start, len);
    s[len] = '\0';