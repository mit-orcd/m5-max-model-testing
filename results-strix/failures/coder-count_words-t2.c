int count_words(const char *s) {
    if (s == NULL) return 0;
    
    int count = 0;
    int in_word = 0;
    
    while (*s) {
        if (*s == ' ') {
            if (in_word) {
                in_word = 0;
            }
        } else {
            if (!in_word) {
                in_word = 1;
                count++;
            }
        }
        s++;
    }
    
    return count;
}