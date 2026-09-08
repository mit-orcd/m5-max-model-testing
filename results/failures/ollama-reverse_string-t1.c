void reverse_string(char *s) {
    size_t len = 0;
    while (s[len])
        len++;
    for (size_t i = 0; i < len / 2; i++) {
        char tmp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = tmp;
    }
}