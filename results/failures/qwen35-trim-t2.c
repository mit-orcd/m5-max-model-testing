#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end;

    // Strip leading whitespace
    while (isspace(*start)) {
        start++;
    }

    // If string is all whitespace or empty
    if (*start == '\0') {
        *start = '\0';
        return;
    }

    // Find end of string
    end = start + strlen(start) - 1;

    // Strip trailing whitespace
    while (end > start && isspace(*end)) {
        end--;
    }

    // Move trimmed content to the beginning
    size_t len = end - start + 1;
    memmove(s, start, len);
    s[len] = '\0';
}