int atoi_strict(const char *s, int *out) {
    if (s == NULL || *s == '\0') return -1;
    
    const char *current = s;
    bool negative = false;
    
    if (*current == '-') {
        negative = true;
        current++;
        if (*current == '\0') return -1;
    }
    
    if (!isdigit(*current)) return -1;
    
    int num = 0;
    while (isdigit(*current)) {
        num = num * 10 + (*current - '0');
        current++;
    }
    
    if (*current != '\0') return -1;
    
    if (negative) num = -num;
    
    *out = num;
    return 0;
}