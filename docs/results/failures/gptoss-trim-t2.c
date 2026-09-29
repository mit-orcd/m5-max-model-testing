#include <string.h>

void trim(char *s) {
    if (!s) return;

    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n')) {
        ++start;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = s + strlen(s) - 1;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        --end;
    }

    size_t len = end - start + 1;
    memmove(s, start, len + 1);  /* includes the terminating null */
}