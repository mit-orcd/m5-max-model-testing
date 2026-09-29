#include <ctype.h>
#include <string.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') {
        return;
    }

    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }

    // Shift the string to the front
    size_t len = strlen(end + 1) + 1;
    memmove(s, start, len);
}