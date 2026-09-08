#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    char *start = s;
    char *end;
    
    // Skip leading whitespace
    while (isspace((unsigned char)*start)) {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of string (pointing to null terminator)
    end = s + strlen(s) - 1;
    
    // Skip trailing whitespace
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    
    // Move trimmed string to beginning
    memmove(s, start, end - start + 1);
    s[end - start + 1] = '\0';
}