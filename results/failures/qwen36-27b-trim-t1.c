#include <string.h>

void trim(char *s) {
    if (s == NULL) return;

    size_t len = strlen(s);
    if (len == 0) return;

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    char *end = s + len - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    size_t new_len = (end >= start) ? (size_t)(end - start) : 0;
    if (new_len < len) {
        memmove(s, start, new_len);
    }
    s[new_len] = '\0';
}