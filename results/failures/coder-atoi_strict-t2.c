int atoi_strict(const char *s, int *out) {
    if (!s) return -1;
    
    const char *p = s;
    int sign = 1;
    int value = 0;
    
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
#include <ctype.h>
int i = 0;
while (p[i] != '\0') {
if (!isdigit((unsigned char)p[i])) {
return -1;
}
i++;
}
if (i == 0) {
return -1;
}
return 0;
} else if (!isdigit((unsigned char)*p)) {
return -1;
}
while (*p != '\0') {
if (!isdigit((unsigned char)*p)) {
return -1;
}
p++;
}
if (p == s) {
return -1;
}
return 0;
}