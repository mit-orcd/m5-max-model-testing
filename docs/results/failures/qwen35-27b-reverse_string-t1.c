#include <stddef.h>

void reverse_string(char *s) {
    if (s == NULL) {
        return;
    }

    size_t left = 0;
    size_t right = 0;

    while (s[right] != '\0') {
        right++;
    }

    right--;

    while (left < right) {
        char temp = s[left];
        s[left] = s[right];
        s[right] = temp;
        left++;
        right--;
    }
}