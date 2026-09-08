#include <stddef.h>

void reverse_string(char *s) {
    size_t len = 0;
    if (s == NULL) {
        return;
    }
    while (s[len] != '\0') {
        len++;
    }
    for (size_t i =  ; i < len /  ; ++) {
        char temp = s[i];
        s[i] = s[len -   -  ];
        s[len -   -  ] = temp;
    }
}