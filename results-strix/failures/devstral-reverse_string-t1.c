void reverse_string(char *s) {
    if (s == NULL) return;
    char *end = s;
    char tmp;
    while (*end != '\0') {
        end++;
    }
    end--;
    while (s < end) {
        tmp = *s;
        *s = *end;
        *end = tmp;
        s++;
        end--;
    }
}