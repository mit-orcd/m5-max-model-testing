void trim(char *s) {
    if (!s) return;
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t' || s[start] == '\n') start++;
    int end = start;
    while (s[end]) end++;
    end--;
    while (end > start && (s[end] == ' ' || s[end] == '\t' || s[end] == '\n')) end--;
    int len = end - start + 1;
    memmove(s, s + start, len);
    s[len] = '\0';
}