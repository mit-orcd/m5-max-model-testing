void reverse_string(char *s) {
    char *l = s, *r = s;
    while (*++r)
        ;
    while (l < r) {
        char t = *l;
        *l = *r;
        *r = t;
        ++l;
        --r;
    }
}