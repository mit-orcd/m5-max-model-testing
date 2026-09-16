void reverse_string(char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    for (size_t i = 0; i < len / 2; ++i) {
        char temp = s[i];
        s[i] = s[len - i - 1];
        s[len - i - 1] = temp;
    }
}