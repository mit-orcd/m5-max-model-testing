void trim(char *s) {
    if (s == NULL || *s == '\0') return;
    
    // Skip leading whitespace
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n') start++;
    
    // If string is all whitespace
    if (*start == '\0') {
        *s = '\0';
        return;
    }
    
    // Remove trailing whitespace
    char *end = start;
    while (*end != '\0') {
        if (*end != ' ' && *end != '\t' && *end != '\n') {
            end++;
        } else {
            char *tail = end + 1;
            while (*tail == ' ' || *tail == '\t' || *tail == '\n') tail++;
            if (*tail == '\0') {
                *end = '\0';
                break;
            } else {
                end++;
            }
        }
    }
    
    // Move trimmed string to beginning
    if (start != s) {
        char *dest = s;
        char *src = start;
        while ((*dest++ = *src++) != '\0');
    }
}