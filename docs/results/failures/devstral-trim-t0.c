#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    // Trim leading whitespace
    while (isspace((unsigned char)*s)) {
        s++;
    }

    if (*s == '\0') {
        return; // String is all whitespace
    }

    // Trim trailing whitespace
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}