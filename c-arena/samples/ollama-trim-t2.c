```c
#include <string.h>

void trim(char *s) {
    char *start = s;
    char *end = s + strlen(s) - 1;

    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;

    while (