int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    unsigned char c;
    uint32_t code;
    unsigned count;
    uint32_t minimum;
    size_t i;

    if (n == 0) {
        return -1;
    }

    c = s[0];

    if (c < 0x80) {
        count = 1;
        code = c;
        minimum = 0;
    } else if (c >= 0xc2 && c <= 0xdf) {
        count = 2;
        code = c & 0x1f;
        minimum = 0x80;
    } else if (c >= 0xe0 && c <= 0xef) {
        count = 3;
        code = c & 0x0f;
        minimum = 0x800;
    } else if (c >= 0xf0 && c <= 0xf4) {
        count = 4;
        code = c & 0x07;
        minimum = 0x10000;
    } else {
        return -1;
    }

    if (n < count) {
        return -1;
    }

    for (i = 1; i < count; ++i) {
        if ((s[i] & 0xc0) != 0x80) {
            return -1;
        }
        code = (code << 6) | (s[i] & 0x3f);
    }

    if (code < minimum || code > 0x10ffff) {
        return -1;
    }

    *cp = code;
    return (int)count;
}