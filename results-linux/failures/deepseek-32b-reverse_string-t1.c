void reverse_string(char *s) {
    size_t len = strlen(s);
    size_t i = 0;
    size_t j = len - 1;
    char temp;
    while (i < j) {
        temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        i++;
        j--;
    }
}