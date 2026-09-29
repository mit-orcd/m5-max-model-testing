void reverse_string(char *s) {
    if (s == NULL) return;
    char *end = s;
    while (*end != '\0') {
        end++;
    }
    end--; // Point to last char before '\0'
    char temp;
    while (s < end) {
        temp = *s;
        *s = *end;
        *end = temp;
        s++;
        end--;
    }
}