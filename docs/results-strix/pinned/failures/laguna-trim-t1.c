#include <ctype.h>

void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Find first non-whitespace character
    char *start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Find last non-whitespace character
    char *end = s;
    char *last_non_ws = s;
    while (*end) {
        if (!isspace((unsigned char)*end)) {
            last_non_ws = end;
        }
        end++;
    }
    
    // If all whitespace or empty string
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Move the trimmed string to the beginning
    char *src = start;
    char *dst = s;
    while (*src) {
        *dst++ = *src++;
    }
    *dst = '\0';
}