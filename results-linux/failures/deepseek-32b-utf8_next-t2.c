int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (s == NULL || n == 0 || cp == NULL) return -1;
    uint32_t c = *s++;
    if ((c & 0x80) == 0) {
        *cp = c;
        return 1;
    }
    size_t len = 0;
    if ((c & 0xE0) == 0xC0) len = 2;
    else if ((c & 0xF0) == 0xE0) len = 3;
    else if ((c & 0xF8) == 0xF0) len = 4;
    else return -1;
    if (n < len) return -1;
    c &= (len == 2) ? 0x1F : (len == 3) ? 0x0F : 0x07;
    for (size_t i = 1; i < len; i++) {
        unsigned char b = s[i];
        if ((b & 0xC0) != 0x80) return -1;
        c <<= 6;
        c |= (b & 0x3F);
    }
    if (c > 0x10FFFF) return -1;
    if (c >= 0xD800 && c <= 0xDFFF) return -1;
    if (len > 1 && (c <= 0x7F || (len > 2 && c <= 0x7FF) || (len > 3 && c <= 0xFFFF))) return -1;
    *cp = c;
    return len;
}