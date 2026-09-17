#include <stddef.h>

void trim(char *s) {
    if (s == NULL) {
        return;
    }

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    char *end = start;
    while (*end != '\0') {
        end++;
    }
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    if (start != s) {
        size_t len = (size_t)(end - start) + 1;
        for (size_t i = 0; i < len; i++) {
            s[i] = start[i];
        }
    } else {
        *(end + 1) = '\0';
    }
}