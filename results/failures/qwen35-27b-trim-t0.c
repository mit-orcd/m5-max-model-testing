#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end;

    while (isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = s + strlen(s) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    end++;
    memmove(s, start, end - start);
    *end = '\0';
}