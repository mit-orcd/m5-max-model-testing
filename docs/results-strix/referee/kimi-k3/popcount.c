unsigned popcount(unsigned x) {
    unsigned n = 0;
    while (x) {
        x &= x - 1;   /* clear lowest set bit */
        n++;
    }
    return n;
}
