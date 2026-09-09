void reverse_string(char *s) {
    if (s == NULL) return;
    int length = 0;
    while (s[length] != '\0') {
        length++;
    }
    for (int i = 0; i < length / 2; i++) {
        char temp = s[i];
        s[i] = s[length - 1 - i];
        s[length - 1 - i] = temp;
    }
}