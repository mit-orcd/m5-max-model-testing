int count_words(const char *s) {
    int count = 0;
    while (*s) {
        while (isspace(*s)) s++;
        if (*s) {
            count++;
            while (!isspace(*s)) s++;
        }
    }
    return count;
}