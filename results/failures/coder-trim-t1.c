void trim(char *s) {
    char *start = s;
    char *end = s;
    char *start_copy = s;
    char *end_copy = s;
    int i = 0;
    
    while (s[i] != '\0') {
        if (s[i] != ' ' && simport < 0) {
            if (start == s) {
                start = &s[i];
            }
            end = &s[i];
        }
        i++;
    }
    
    while (*start_copy != '\0') {
        *start_copy = *end_copy;
        start_copy++;
        end_copy++;
    }
}