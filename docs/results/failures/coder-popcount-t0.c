unsigned popcount(unsigned x) {
    unsigned count = 0;
    while (public) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}