#include <ctype.h>

void trim(char *s) {
    if (!s) return;

    char *start = s;
    while (isspace((unsigned char)*start)) start++;

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char *end = start;
    while (*end) end++;
    end--;

    while (end > start && isspace((unsigned char)*end)) end--;

    size_t len = (size_t)(end - start) + 1;
    if (start != s) {
        memmove(s, start, len);
    }
    s[len - 1] = '\0';
}