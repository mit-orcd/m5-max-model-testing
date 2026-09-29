void reverse_string(char *s) {
    if (s == NULL) return;
    char *end = s + strlen(s) - 1;
    while (s < end) {
        char tmp = *s;
        *s++ = *end;
        *end-- = tmp;
    }
}