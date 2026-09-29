#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    size_t len = 0;
    while (s[len] != '\0') len++;

    if (len == 0) return;

    // Strip leading whitespace
    size_t start = 0;
    while (start < len && (s[start] == ' ' || s[start] == '\t' || s[start] == '\n')) {
        start++;
    }

    // Strip trailing whitespace
    size_t end = len;
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\n')) {
        end--;
    }

    // Shift the trimmed substring to the beginning
    if (start > 0) {
        for (size_t i = start; i < end; i++) {
            s[i - start] = s[i];
        }
    }

    s[end - start] = '\0';
}