#include <ctype.h>
#include <string.h>

void trim(char *s) {
    if (!s) return;

    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        if (*start == ' ' || *start == '\t' || *start == '\n') {
            start++;
        } else {
            break;
        }
    }

    if (*start == '\0') {
        s[0] = '\0';
        return;
    }

    size_t len = strlen(start);
    char *end = start + len - 1;

    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }

    end[1] = '\0';

    if (start != s) {
        memmove(s, start, (size_t)(end - start + 1));
    }
}