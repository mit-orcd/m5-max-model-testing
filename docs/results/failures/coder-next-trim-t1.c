void trim(char *s) {
    if (!s) return;
    
    char *start = s;
    char *end;
    
    // Skip leading whitespace
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }
    
    // If string is all whitespace or empty
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of non-whitespace content
    end = start;
    while (*end != '\0') {
        end++;
    }
    end--; // Point to last character
    
    // Skip trailing whitespace
    while (end > start && (*end == ' ' || *end == '\t' || *end == '\n')) {
        end--;
    }
    
    // Move trimmed string to beginning
    memmove(s, start, end - start + 1);
    s[end - start + 1] = '\0';
}