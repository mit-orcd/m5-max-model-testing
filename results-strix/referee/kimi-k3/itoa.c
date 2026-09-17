void itoa(int value, char *buf) {
    char tmp[12];
    int i = 0, j = 0;
    int neg = value < 0;
    /* Unsigned arithmetic handles INT_MIN safely. */
    unsigned int u = neg ? 0u - (unsigned int)value : (unsigned int)value;
    do {
        tmp[i++] = (char)('0' + u % 10);
        u /= 10;
    } while (u);
    if (neg) buf[j++] = '-';
    while (i > 0) buf[j++] = tmp[--i];
    buf[j] = '\0';
}
