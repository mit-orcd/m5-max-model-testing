void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Find start of non-whitespace
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') {
        start++;
    }
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Find end of non-whitespace
    char *end = start;
    while (*end != '\0' && !(*end == ' ' || *end == '\t' || *end == '\n')) {
        end++;
    }
    while (end > start && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) {
        end--;
    }
    
    // Move trimmed string to beginning
    memmove(s, start, end - start);
    s[end - start] = '\0';
}