#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    char *end = start + (start - s);
    /* Find the last non-whitespace character */
    char *p = s;
    while (*p) p++;
    while (p > start && isspace((unsigned char)*(p - 1))) {
        p--;
    }

    /* Move the trimmed content to the beginning */
    size_t len = (size_t)(p - start);
    for (size_t i = 0; i < len; i++) {
        s[i] = start[i];
    }
    s[len] = '\0';
}