```c
void trim(char *s) {
    char *p = s, *end;

    while (*p == ' ' || *p == '\t' || *p == '\n') {
        ++p;
    }

    end = p;
    while (*end) {
        ++end;
    }

    while (end > p && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\n')) {
        --end;
    }

    *end