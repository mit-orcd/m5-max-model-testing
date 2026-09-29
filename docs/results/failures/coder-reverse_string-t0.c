:
    void reverse_string(char *s) {
        char *start = s;
        char *end = s;
        
        // Find the end of the string
        while (*end != '\0') {
            end++;
        }
        
        // Reverse the string in place
        end--;
        while (start < end) {
            char temp = *start;
            *start = *end;
            *end = temp;
            start++;
            end--;
        }
    }