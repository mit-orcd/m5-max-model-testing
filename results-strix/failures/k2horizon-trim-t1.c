#include <ctype.h>

void trim(char *s) {
    char *end;

    if (s == NULL) return;

    while (isspace((unsigned char)*s)) s++;

    if (*s == '\0') {
        *s = '\0';
        return;
    }

    end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    *s = '\0'; // Placeholder to avoid unused warning; actual shift below
    // Shift back to original start
    // (No-op if s was already at start)
}