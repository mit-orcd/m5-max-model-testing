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
    
    // Find end of non-whitespace
    char *end = start;
    while (*end && !isspace((unsigned char)*end)) {
        end++;
    }
    // Move past last non-whitespace character
    while (*end) {
        end++;
    }
    // Back up to last non-whitespace character
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    
    // Move trimmed string to beginning
    size_t len = end - start;
    memmove(s, start, len);
    s[len] = '\0';
}