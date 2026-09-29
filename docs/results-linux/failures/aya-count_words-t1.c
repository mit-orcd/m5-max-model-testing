int count_words(const char *s) {
    int count = 0;
    while (*s) {
        while (*s && isspace(*s)) s++;
        if (*s) {
            count++;
            while (*s && !isspace(*s)) s++;
        }
    }
    return count;
}