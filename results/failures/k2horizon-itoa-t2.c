void itoa(int value, char *buf) {
    char *p = buf;
    int negative = value < 0;

    if (negative) {
        if (value == INT_MIN) {
            unsigned int u = (unsigned int)(value - 1) + 1U;
            *p++ = '-';
            do {
                *p++ = (char)('0' + (u % 10));
                u /= 10;
            } while (u);
        } else {
            int v = -value;
            *p++ = '-';
            do {
                *p++ = (char)('0' + (v % 10));
                v /= 10;
            } while (v);
        }
    } else {
        unsigned int u = (unsigned int)value;
        do {
            *p++ = (char)('0' + (u % 10));
            u /= 10;
        } while (u);
    }

    *p = '\0';

    char *start = buf;
    char *end = p - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}