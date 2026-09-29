#include <ctype.h>

void trim(char *s) {
    if (*s == '\0') return;

    // Trim leading whitespace
    while (isspace((unsigned char)*s)) {
        s++;
    }

    // Trim trailing whitespace
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0';
}