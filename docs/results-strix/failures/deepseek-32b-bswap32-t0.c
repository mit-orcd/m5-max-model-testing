uint32_t bswap32(uint32_t x) {
    return ((x & 0xFF) << 24) | ((x >> 8) & 0xFF) << 16 | ((x >> 16) & 0xFF) << 8 | (x >> 24) & 0xFF;
}