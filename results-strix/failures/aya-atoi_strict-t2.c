int atoi_strict(const char *s, int *out) {
    int neg = 0, i;
    for (i = 0; isspace(s[i]); i++)
        ;
    if (s[i] == '+')
        i++;
    else if (s[i] == '-') {
        neg = 1;
        i++;
    }
    if (isdigit(s[i])) {
        int x = 0;
        while (isdigit(s[i])) {
            x = x * 10 + (s[i++] - '0');
        }
        if (i == strlen(s)) {
            *out = neg ? -x : x;
            return 0;
        }
    }
    return -1;
}