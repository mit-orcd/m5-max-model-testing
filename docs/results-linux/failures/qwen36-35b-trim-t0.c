#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    // Strip leading whitespace
    while (isspace((unsigned char)*s)) {
        s++;
    }

    // Strip trailing whitespace
    char *end = s;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (end > s && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';

    // Move the trimmed string to the beginning if leading spaces were removed
    if (s != end + 1) {
        char *dst = s;
        char *src = s;
        while (*src) {
            *dst++ = *src++;
        }
        *dst = '\0';
    }
}