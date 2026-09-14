#include <stddef.h>

void trim(char *s) {
    if (s == NULL) {
        return;
    }

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = s + (size_t)(start - s);
    while (*end != '\0') {
        end++;
    }

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    *(end + 1) = '\0';

    if (start != s) {
        size_t len = (size_t)(end - start + 1);
        memmove(s, start, len);
    }
}