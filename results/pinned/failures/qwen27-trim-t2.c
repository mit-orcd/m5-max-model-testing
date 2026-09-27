#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    if (start != s) {
        size_t len = (size_t)(end - start + 1);
        memmove(s, start, len);
        s[len] = '\0';
    } else {
        end[1] = '\0';
    }
}