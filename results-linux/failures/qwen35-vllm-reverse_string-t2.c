#include <string.h>

void reverse_string(char *s) {
    if (s == NULL) return;
    
    size_t len = strlen(s);
    size_t i = 0;
    size_t j = len - 1;
    
    while (i < j) {
        char temp = s[i];
        s[i] = s[j];
        s[j] = temp;
        i++;
        j--;
    }
}