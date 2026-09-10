void reverse_string(char *s) {
    if (s == NULL) return;
    char *start = s;
    char *end = s;
    while (*end) end++;
    end--;
    while (start < end) {
        char tmp = *start;
        *start = *end;
        *start++ = *end;
        *end-- = tmp;
    }
}