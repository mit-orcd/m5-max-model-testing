void reverse_string(char *s) {
    if (s == NULL) return;
    char *left = s;
    char *right = s;
    while (*right != '\0') right++;
    right--;
    while (left < right) {
        char temp = *left;
        *left = *right;
        *right = temp;
        left++;
        right--;
    }
}