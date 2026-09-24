void reverse_string(char *s) {
    if (s == NULL) return;
    char *end = s;
    while (*end != '\0') end++;
    end--;
    char temp;
    while (s < end) {
        temp = *s;
        *s++ = *end;
        *end-- = temp;
    }
}