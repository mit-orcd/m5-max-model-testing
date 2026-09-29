void trim(char *s) {
    char *start = s;
    while (*start == ' ' || *start == '\t' || *start == '\n')
        start++;
    char *end = start;
    while (*end)
        end++;
    end--;
    while (end >= start && (*end == ' ' || *end == '\t' || *end == '\n'))
        end--;
    int len = end - start + 1;
    for (int i = 0; i <= len; i++)
        s[i] = start[i];
}