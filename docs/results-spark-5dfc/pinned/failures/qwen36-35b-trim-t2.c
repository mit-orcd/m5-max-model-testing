#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    // Strip leading whitespace
    char *start = s;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // Strip trailing whitespace
    char *end = start + strlen(start) - 1;
    while (end >= start && isspace((unsigned char)*end)) {
        end--;
    }

    // Move trimmed string to the beginning if necessary
    if (start != s) {
        memmove(s, start, (size_t)(end - start + 2)); // +2 for null terminator
    } else {
        // If no leading whitespace, just truncate trailing
        *(end + 1) = '\0';
    }
}