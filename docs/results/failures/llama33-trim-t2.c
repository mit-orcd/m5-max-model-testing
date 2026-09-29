#include <ctype.h>

void trim(char *s) {
    char *start = s;
    char *end = s;
    while (*end) {
        end++;
    }
    end--;
    while (start < end && isspace(*start)) {
        start++;
    }
    while (end > start && isspace(*end)) {
        *end = '\0';
        end--;
    }
    int len = end - start + 1;
    memmove(s, start, len);
}