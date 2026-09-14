static inline unsigned popcount(unsigned x)
{
    /* Brian Kernighan's algorithm (fast for sparse bits) */
    unsigned count = 0;
    while (x)
    {
        x &= x - 1;   /* clear the least significant set bit */
        ++count;
    }
    return count;
}