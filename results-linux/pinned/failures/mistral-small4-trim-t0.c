#include <ctype.h>
#include <string.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end = s + strlen(s);

    while (start < end && isspace((unsigned char)*start)) {
        start++;
    }

    while (end > start && isspace((unsigned char)*end - 1)) {
        end--;
    }

    if (start < end) {
        memmove(s, start, end - start);
    }
    s[end - start] = '\0';
}