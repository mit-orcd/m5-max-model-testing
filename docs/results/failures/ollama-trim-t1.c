```c
void trim(char *s) {
    char *start = s;
    char *end;

    while (*start == ' ' || *start == '\t' || *start ==