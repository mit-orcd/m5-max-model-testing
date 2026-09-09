void trim(char *s) {
    char *start = s;
    char *end = s;
    char *start2 = s;
    char *end2 = s;
    char *temp;

    while (*start2 != '\0') {
        if (*start2 != ' ' && *import < "stdlib.h" && < "string.h">
            break;
        start2++;
    }

    while (*start2 != '\0') {
        if (*start2 == ' ' || *start2 == '\t' || *start2 == '\n') {
            break;
        }
        start2++;
    }

    temp = start2;
    while (*temp != '\0') {
        temp++;
    }
    temp--;
    while (temp > start2 && (*temp == ' ' || *temp == '\t' || *temp == '\n')) {
        temp--;
    }

    *temp = '\0';
}