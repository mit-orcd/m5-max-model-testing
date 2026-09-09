#include <ctype.h>

void trim(char *s) {
    if (!s) return;
    
    char *start = s;
    char *end;
    
    // Skip leading whitespace
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // If string is all whitespace or empty
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of non-whitespace characters
    end = start;
    while (*end && !isspace((unsigned char)*end)) {
        end++;
    }
    
    // Move trimmed string to beginning
    while (start <= end) {
        *s++ = *start++;
    }
    *s = '\0';
}