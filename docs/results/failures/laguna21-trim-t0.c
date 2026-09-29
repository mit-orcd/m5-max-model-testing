#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') {
        return;
    }

    // Find the start of the string (skip leading whitespace)
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // If the string is all whitespace, set it to empty
    if (*start == '\0') {
        *s = '\0';
        return;
    }

    // Find the end of the string (skip trailing whitespace)
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Calculate the length of the trimmed string
    size_t len = end - start + 1;

    // Move the trimmed string to the beginning
    memmove(s, start, len);

    // Null-terminate the string
    s[len] = '\0';
}