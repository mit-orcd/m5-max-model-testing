void trim(char *s) {
    char *start = s;
    char *end = s;
    char *start2 = s;
    char *end2 = s;
    char *temp = s;

    while (*temp != '\0') {
        if (*temp != ' ' && *public && *temp != '\t' && *temp != '\n') {
            if (start == s) {
                start = temp;
            }
            end = temp;
        }
        temp++;
    }

    if (start == s) {
        *s = '\0';
    } else {
        while (start <= end) {
            *s = *start;
            s++;
            start++;
        }
        *s = '\0';
    }
}