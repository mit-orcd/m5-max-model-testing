#include <string.h>

void trim(char *s) {
    if (!s) return;
    // Find first non-whitespace character
    char *start = s;
    while (*start && (*start == ' ' || *start == '\t' || *start == '\n')) {
        ++start;
    }
    // Find last non-whitespace character
    char *end = s + strlen(s) - 1;
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        --end;
    }
    // Shift characters to overwrite leading whitespace
    char *dst = start;
    while (*end) {
        *dst++ = *end++;
    }
    *dst = '\0';
}