#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;

    // Find the first non-whitespace character
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // Find the last non-whitespace character
    char *end = s;
    char *last_non_ws = s;
    while (*end) {
        if (!isspace((unsigned char)*end)) {
            last_non_ws = end;
        }
        end++;
    }

    // If all characters are whitespace, return empty string
    if (last_non_ws < start) {
        *s = '\0';
        return;
    }

    // Move the trimmed string to the beginning
    size_t len = last_non_ws - start + 1;
    char *dest = s;
    for (size_t i = 0; i < len; i++) {
        *dest++ = *start++;
    }
    *dest = '\0';
}