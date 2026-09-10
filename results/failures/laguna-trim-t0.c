#include <ctype.h>

void trim(char *s) {
    if (!s) return;
    
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    char *end = start;
    char *p = start;
    while (*p) {
        p++;
    }
    p--;
    while (p > start && isspace((unsigned char)*p)) {
        p--;
    }
    end = p + 1;
    
    size_t len = end - start;
    memmove(s, start, len);
    s[len] = '\0';
}