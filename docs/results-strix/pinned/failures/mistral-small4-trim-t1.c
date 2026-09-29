#include <ctype.h>
#include <string.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end = s + strlen(s);

    while (start < end && isspace((unsigned char)*start)) {
        start++;
    }

    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    if (start < end) {
        size_t len = end - start + 1;
        memmove(s, start, len);
        s[len] = '\0';
    } else {
        s[0] = '\0';
    }
}