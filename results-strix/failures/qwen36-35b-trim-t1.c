#include <ctype.h>

void trim(char *s) {
    char *start = s;
    char *end;
    char *p;

    while (isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    end = start + 1;
    while (*end != '\0') {
        end++;
    }
    end--;

    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    p = start;
    while (start <= end) {
        *p++ = *start++;
    }
    *p = '\0';
}