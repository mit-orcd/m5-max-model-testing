#include <stddef.h>

void trim(char *s) {
    if (s == NULL) {
        return;
    }

    // Find first non-whitespace character
    char *start = s;
    while (*start != '\0') {
        if (*start != ' ' && *start != '\t' && *start != '\n') {
            break;
        }
        start++;
    }

    // If the string is all whitespace, make it empty
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    // Find last non-whitespace character
    char *end = s + __builtin_strlen(s) - 1;
    while (end >= start) {
        if (*end != ' ' && *end != '\t' && *end != '\n') {
            break;
        }
        end--;
    }

    // Move the trimmed portion to the beginning
    size_t len = (size_t)(end - start) + 1;
    if (start != s) {
        for (size_t i = 0; i <= len; i++) {
            s[i] = start[i];
        }
    }
}