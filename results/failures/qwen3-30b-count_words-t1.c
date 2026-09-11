int count_words(const char *s) {
    if (s == NULL) {
        return 0;
    }
    while (*s == ' ') {
        s++;
    }
    int count = 0;
    while (*s) {
        count++;
        while (*s && *s != ' ') {
            s++;
        }
        while (*s == ' ') {
            s++;
        }
    }
    return count;
}