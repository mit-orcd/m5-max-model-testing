void reverse_string(char *s) {
    if (s == NULL) return;
    char *start = s;
    char *end = s;
    while (*end != '\0') end++;
    end--;
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *end = tmp;
        start++;
        end--;
    }
}