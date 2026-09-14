#include <stddef.h>

void trim(char *s) {
    if (s == NULL) {
        return;
    }

    /* Strip leading whitespace */
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    /* Strip trailing whitespace */
    char *end = start;
    while (*end != '\0') {
        end++;
    }
    while (end > start && *(end - 1) == ' ' && *(end - 1) == '\t' && *(end - 1) == '\n') {
        end--;
    }

    /* Shift remaining characters to the beginning */
    size_t len = (size_t)(end - start);
    for (size_t i = 0; i < len; i++) {
        s[i] = start[i];
    }
    s[len] = '\0';
}