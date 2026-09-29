#include <stddef.h>

void reverse_string(char *s) {
    if (s == NULL) return;

    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }

    size_t left = 0;
    size_t right = len - 1;
    while (left < right) {
        char temp = s[left];
        s[left] = s[right];
        s[right] = temp;
        left++;
        right--;
    }
}