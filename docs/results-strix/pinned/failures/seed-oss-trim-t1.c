```c
#include <string.h>

void trim(char *s) {
    if (s == NULL) {
        return;
    }

    char *start = s;
    while (*start != '\0' && (*start == ' ' || *start == '\t' || *start == '\n')) {
        start++;
    }

    if (*start == '\0') {
        *s = '\0';
        return;
    }

    char