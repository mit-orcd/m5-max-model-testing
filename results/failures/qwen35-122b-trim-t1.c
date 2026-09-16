#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') {
        return;
    }

    char *start = s;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    *(end + 1) = '\0';

    if (start != s) {
        char *dst = s;
        while (dst != end + 1) {
            *dst++ = *start++;
        }
        *dst = '\0';
    }
}