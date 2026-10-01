void reverse_string(char *s) {
    if (s == NULL) return;
    
    char *start = s;
    char *end = s;
    
    // Find the end of the string
    while (*end != '\0') {
        end++;
    }
    end--; // Move back to the last character
    
    // Reverse the string in place
    while (start < end) {
        char temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}