#include <stddef.h>

void trim(char *s) {
    if (s == NULL) return;

    /* Strip leading whitespace */
    while (*s == ' ' || *s == '\t' || *s == '\n') {
        s++;
    }

    /* If the string is now empty, we're done */
    if (*s == '\0') return;

    /* Find the end of the string */
    char *end = s;
    while (*end != '\0') {
        end++;
    }
    end--; /* points to last character */

    /* Strip trailing whitespace */
    while (end >= s && (*end == ' ' || *end == '\t' || *end == '\n')) {
        *end = '\0';
        end--;
    }
}