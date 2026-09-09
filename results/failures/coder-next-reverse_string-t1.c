void reverse_string(char *s) {
    if (s == NULL) return;
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    if (len <= 1) return;
    for (size_t i = 0; i < len / 2; i++) {
        char temp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = temp;
    }
}