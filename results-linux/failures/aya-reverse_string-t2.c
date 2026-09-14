void reverse_string(char *s) {
    char *end = s + strlen(s) - 1;
    char *start = s;
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}