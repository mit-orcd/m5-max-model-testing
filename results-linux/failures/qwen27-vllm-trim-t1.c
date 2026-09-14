#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }

    char *end = start + __builtin_strlen(start);
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    end++;

    while (*start != '\0') {
        *s++ = *start++;
    }
    *s = '\0';
}