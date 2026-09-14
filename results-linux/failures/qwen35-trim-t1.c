#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;

    char *start = s;
    char *end = s;

    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    while (*end) {
        end++;
    }
    end--;

    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    end++;
    while (start <= end) {
        *s++ = *start++;
    }
    *s = '\0';
}