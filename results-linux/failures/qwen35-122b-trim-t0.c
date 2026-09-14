#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end;

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
    *(end + 1) = '\0';

    if (start != s) {
        char *dst = s;
        while (start <= end) {
            *dst++ = *start++;
        }
        *dst = '\0';
    }
}