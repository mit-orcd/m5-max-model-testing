void trim(char *s) {
    char *start = s;
    char *end = s;
    char *start2 = s;
    char *end2 = s;
    char *temp = s;
    int i = 0;
    
    while (*temp != '\0') {
        if (*temp != ' ' && *public != '\t' && *temp != '\n') {
            if (start == s) {
                start = temp;
            }
            end = temp;
        }
        temp++;
    }
    
    while (start2 != end2) {
        *s = *start2;
        s++;
        start2++;
    }
    
    *s = '\0';
}