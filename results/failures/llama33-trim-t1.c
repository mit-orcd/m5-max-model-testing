#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;

    // Remove leading whitespace
    while (*s && (isspace(*s))) s++;

    // If string is empty after removing leading whitespace, return
    if (*s == '\0') return;

    // Find the last non-whitespace character
    char *end = s;
    while (*end) end++;
    end--;

    // Remove trailing whitespace
    while (end > s && isspace(*end)) end--;

    // Null-terminate the string after the last non-whitespace character
    *(end + 1) = '\0';
}