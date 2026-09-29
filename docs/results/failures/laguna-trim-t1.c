#include <string.h>

void trim(char *s) {
    if (!s) return;
    
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }
    
    char *end = s + strlen(s) - 1;
    while (end >= s && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    
    size_t len = end - start + 1;
    memmove(s, start, len);
    s[len] = '\0';
}