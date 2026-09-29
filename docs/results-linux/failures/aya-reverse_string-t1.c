void reverse_string(char *s) {
    char *start = s, *end = s + strlen(s) - 1;
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}