#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Find start of trimmed string (skip leading whitespace)
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of trimmed string (skip trailing whitespace)
    char *end = start;
    while (*end && !isspace((unsigned char)*end)) {
        end++;
    }
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    
    // Move trimmed string to beginning of buffer
    size_t len = end - start;
    memmove(s, start, len);
    s[len] = '\0';
}