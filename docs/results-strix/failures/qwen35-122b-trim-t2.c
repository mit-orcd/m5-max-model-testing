#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        s[0] = '\0';
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    size_t len = (size_t)(end - start + 1);
    for (size_t i = 0; i < len; i++) {
        s[i] = start[i];
    }
    s[len] = '\0';
}