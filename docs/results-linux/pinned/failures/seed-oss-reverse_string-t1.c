void reverse_string(char *s) {
    if (s == NULL) {
        return;
    }
    int len = 0;
    while (s[len] != '\0') {
        len++;
    }
    for (int i = 0; i < len / 2; i++) {
        char temp = s[i];
        s[i] = s[len - 1 - i];
        s[len - 1 - i] = temp;
    }
}