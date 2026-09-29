#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }

    char *end = start + __builtin_strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }

    if (start != s) {
        size_t len = (size_t)(end - start);
        for (size_t i = 0; i < len; i++) {
            s[i] = start[i];
        }
        s[len] = '\0';
    } else {
        *end = '\0';
    }
}