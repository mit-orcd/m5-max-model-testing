void itoa(int value, char *buf) {
    int i = 0;
    int neg = 0;

    if (value < 0) {
        neg = 1;
        value = -(value + 1);
    }

    do {
        buf[i++] = (char)('0' + (value % 10));
        value /= 10;
    } while (value > 0);

    if (neg) {
        buf[i++] = (char)('0' + 1);
        i--;
    }

    if (neg) {
        buf[i++] = '-';
    }
    buf[i] = '\0';
}