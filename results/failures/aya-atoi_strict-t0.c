int atoi_strict(const char *s, int *out) {
    int neg = 0, i;
    for (i = 0; s[i] == ' '; i++)
        ;
    if (s[i] == '-') {
        neg = 1;
        i++;
    }
    if (s[i] < '0' || s[i] > '9')
        return -1;
    for (; s[i] >= '0' && s[i] <= '9'; i++)
        ;
    if (s[i] != '\0')
        return -1;
    *out = neg ? -(int)strtol(s, NULL, 10) : strtol(s, NULL, 10);
    return 0;
}