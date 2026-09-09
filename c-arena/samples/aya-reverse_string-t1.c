void reverse_string(char *s) {
    int i, j, n = strlen(s);
    for (i = 0, j = n - 1; i < j; i++, j--) {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
    }
}