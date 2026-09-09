void reverse_string(char *s) {
    if (s == NULL)
        return;

    int len = 0;
    while (s[len] != '\0') {
        len++;
    }

    int start = 0;
    int end = len - 1;
    while (start < end) {
        char tmp = s[start];
        s[start] = s[end];
        s[end] = tmp;
        start++;
        end--;
    }
}