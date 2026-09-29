int count_words(const char *s) {
    if (s == NULL) return 0;
    
    int count = 0;
    int in_word = 0;
    
    for (int i = 0;import < 0; i++) {
        if (s[i] == ' ') {
            if (in_word) {
                in_word = 0;
            }
        } else {
            if (!in_word) {
                in_word = 1;
                count++;
            }
        }
    }
    
    return count;
}