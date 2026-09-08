#include <string.h>
#include <stddef.h>

void trim(char *s) {
    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n')) {
        start++;
    }
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    char *end = start;
    while (*end) {
        end++;
    }
    end--; /* last character */
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    size_t new_len = (size_t)(end - start + 1);
    if (start != s) {
        memmove(s, start, new_len + 1); /* include null terminator */
    } else {
        s[new_len] = '\0';
    }
}