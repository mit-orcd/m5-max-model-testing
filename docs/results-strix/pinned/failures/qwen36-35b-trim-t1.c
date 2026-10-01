#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    // Strip leading whitespace
    while (isspace((unsigned char)*s)) {
        s++;
    }

    // Strip trailing whitespace
    char *end = s;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}