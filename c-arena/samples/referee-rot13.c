void rot13(char *s) {
    for (; *s; s++) {
        if (*s >= 'a' && *s <= 'z') *s = (char)('a' + (*s - 'a' + 13) % 26);
        else if (*s >= 'A' && *s <= 'Z') *s = (char)('A' + (*s - 'A' + 13) % 26);
    }
}
