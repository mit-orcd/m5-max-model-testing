#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;

    char *end;
    char *start = s;

    while (isspace((unsigned char)*start)) {
        start++;
    }

    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }

    if (start != s) {
        memmove(s, start, end - start + 2);
    } else {
        *(end + 1) = '\0';
    }
}