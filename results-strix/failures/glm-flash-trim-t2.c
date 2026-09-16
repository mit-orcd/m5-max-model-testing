#include <ctype.h>
#include <stddef.h>

void trim(char *s) {
    if (s == NULL) return;

    size_t len = 0;
    while (s[len] != '\0') len++;

    size_t i = 0;
    while (i < len && isspace((unsigned char)s[i])) i++;

    size_t j = len;
    while (j > i && isspace((unsigned char)s[j - 1])) j--;

    size_t size = j - i;
    if (i > 0) {
        memmove(s, s + i, size);
    }
    s[size] = '\0';
}