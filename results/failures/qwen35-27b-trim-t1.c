#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = start;
    while (*end) {
        end++;
    }

    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }

    size_t len = end - start;
    memmove(s, start, len);
    s[len] = '\0';
}