#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (s == NULL) {
        return;
    }

    char *start = s;
    char *end = s + strlen(s) - 1;

    // Strip leading whitespace
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // Strip trailing whitespace
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    // Copy the trimmed part back to the original buffer
    size_t len = (size_t)(end - start) + 1;
    memmove(s, start, len);
    s[len - 1] = '\0';
}