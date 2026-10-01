void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Find start of trimmed string
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of trimmed string
    char *end = start;
    while (*end != '\0') {
        end++;
    }
    end--;
    
    // Move backwards to find last non-whitespace character
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    
    // Copy trimmed string to beginning of original buffer
    memmove(s, start, end - start + 1);
    s[end - start + 1] = '\0';
}