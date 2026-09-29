#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Find start of trimmed string
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of trimmed string
    char *end = start;
    while (*end && !isspace((unsigned char)*end)) {
        end++;
    }
    while (end > start && isspace((unsigned char)*(end - 1))) {
        end--;
    }
    
    // Move trimmed string to beginning
    memmove(s, start, end - start);
    s[end - start] = '\0';
}