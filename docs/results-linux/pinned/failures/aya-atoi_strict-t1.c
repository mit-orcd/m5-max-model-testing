int atoi_strict(const char *s, int *out) {
    int neg = 0, i;
    if (s[0] == '-') {
        neg = 1;
        s++;
    }
    for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
        ;
    if (i == 0 || s[i] != '\0')
        return -1;
    *out = neg ? -atoi(s) : atoi(s);
    return 0;
}