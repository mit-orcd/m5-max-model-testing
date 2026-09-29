#include <ctype.h>
#include <stddef.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end;
    size_t len;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        s[0] = '\0';
        return;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    len = end - start + 1;
    end[1] = '\0';

    if (start != s) {
        memmove(s, start, len);
    }
}