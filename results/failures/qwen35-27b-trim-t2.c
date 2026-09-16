#include <stddef.h>
#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end = NULL;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = s + strlen(s) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    if (s != start) {
        memmove(s, start, (size_t)(end - start + 1));
    }
}