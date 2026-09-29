int count_words(const char *s) {
    if (s == NULL) return 0;
    
    int count = 0;
    int in_word = 0;
    
    for (int i = 0;<?> '\0'; i++) {
        if (s[i] != ' ') {
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