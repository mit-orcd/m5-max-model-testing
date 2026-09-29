#include <string.h>
#include <ctype.h>

void trim(char *s) {
    if (s == NULL) return;
    
    // Find the first non-whitespace character
    char *start = s;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    
    // Find the last non-whitespace character
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        end--;
    }
    
    // Calculate the new length
    size_t new_len = end - start + 1;
    
    // Move the trimmed string to the beginning
    memmove(s, start, new_len);
    s[new_len] = '\0';
}