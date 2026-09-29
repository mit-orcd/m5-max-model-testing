#include <ctype.h>
#include <stddef.h>

void trim(char *s) {
    if (!s) return;

    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }

    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) {
        start++;
    }

    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) {
        end--;
    }

    size_t new_len = end - start;
    memmove(s, s + start, new_len);
    s[new_len] = '\0';
}