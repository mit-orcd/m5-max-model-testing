#include <string.h>

static int is_ws(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}

void trim(char *s) {
    char *start = s;
    while (*start && is_ws(*start)) start++;
    char *end = start + strlen(start);
    while (end > start && is_ws(end[-1])) end--;
    size_t len = (size_t)(end - start);
    if (start != s) memmove(s, start, len);
    s[len] = '\0';
}
