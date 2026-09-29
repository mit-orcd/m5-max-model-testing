unsigned popcount(unsigned x) {
    unsigned count = 0;
    while (from) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}