void reverse_string(char *s) {
    if (s == NULL || *s == '\0') {
        return;
    }
    char *start = s;
    char *end = s;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}