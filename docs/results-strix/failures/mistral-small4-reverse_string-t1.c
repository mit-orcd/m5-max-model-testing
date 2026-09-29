void reverse_string(char *s) {
    if (s == NULL) return;
    char *end = s;
    while (*end != '\0') end++;
    end--;
    while (s < end) {
        char temp = *s;
        *s++ = *end;
        *end-- = temp;
    }
}