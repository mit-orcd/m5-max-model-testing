int count_words(const char *s) {
    int count = 0;
    int in_word = 0;
    if (s == NULL) return 0;
    for (const char *p = s; *p; p++) {
        if (*p != ' ') {
            if (!in_word) {
                count++;
                in_word = 1;
            }
        } else {
            in_word = 0;
        }
    }
    return count;
}