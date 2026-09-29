#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Skip leading whitespace
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of non-whitespace content
    char *end = start;
    while (*end && !isspace((unsigned char)*end)) {
        end++;
    }
    
    // Move trimmed string to the beginning
    memmove(s, start, end - start);
    s[end - start] = '\0';
}