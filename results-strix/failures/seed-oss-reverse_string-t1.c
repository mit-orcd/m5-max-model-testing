void reverse_string(char *s) {
    if (s == NULL) {
        return;
    }
    char *end = s;
    while (*end != '\0') {
        end++;
    }
    if (end == s) {
        return;
    }
    end--;
    char *start = s;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}