int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    unsigned c1;
    unsigned c2;
    unsigned c3;
    unsigned c4;

    if (n == 0)
        return -1;

    c1 = s[0];

    if (c1 < 0x80) {
        *cp = (uint32_t)c1;
        return 1;
    }

    if (c1 >= 0xC2 && c1 <= 0xDF) {
        if (n < 2)
            return -1;
        c2 = s[1];
        if ((c2 & 0xC0) != 0x80)
            return -1;
        *cp = ((uint32_t)(c1 & 0x1F) << 6) |
              (uint32_t)(c2 & 0x3F);
        return 2;
    }

    if (c1 >= 0xE0 && c1 <= 0xEF) {
        if (n < 3)
            return -1;
        c2 = s[1];
        c3 = s[2];
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80)
            return -1;
        if (c1 == 0xE0 && c2 < 0xA0)
            return -1;
        if (c1 == 0xED && c2 > 0x9F)
            return -1;
        *cp = ((uint32_t)(c1 & 0x0F) << 12) |
              ((uint32_t)(c2 & 0x3F) << 6) |
              (uint32_t)(c3 & 0x3F);
        return 3;
    }

    if (c1 >= 0xF0 && c1 <= 0xF4) {
        if (n < 4)
            return -1;
        c2 = s[1];
        c3 = s[2];
        c4 = s[3];
        if ((c2 & 0xC0) != 0x80 || (c3 & 0xC0) != 0x80 ||
            (c4 & 0xC0) != 0x80)
            return -1;
        if (c1 == 0xF0 && c2 < 0x90)
            return -1;
        if (c1 == 0xF4 && c2 > 0x8F)
            return -1;
        *cp = ((uint32_t)(c1 & 0x07) << 18) |
              ((uint32_t)(c2 & 0x3F) << 12) |
              ((uint32_t)(c3 & 0x3F) << 6) |
              (uint32_t)(c4 & 0x3F);
        return 4;
    }

    return -1;
}