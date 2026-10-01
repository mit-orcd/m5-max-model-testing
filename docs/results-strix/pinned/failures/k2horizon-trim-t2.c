#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    // Skip leading whitespace
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }

    // Find end of string
    char *end = s;
    while (*end) {
        end++;
    }

    // Skip trailing whitespace
    while (end > s && isspace((unsigned char)end[-1])) {
        end--;
    }

    // Null-terminate at the new end
    *end = '\0';

    // Copy the trimmed string back to the original start
    char *dest = s;
    while (*s) {
        *dest++ = *s++;
    }
    *dest = '\0';
}