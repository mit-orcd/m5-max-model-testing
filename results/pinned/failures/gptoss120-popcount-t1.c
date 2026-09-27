/* Count the number of set bits in an unsigned integer (C11) */
static inline unsigned popcount(unsigned x)
{
    /* 32‑bit version of the "parallel" popcount algorithm */
    x = x - ((x >> 1) & 0x55555555u);
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;
    x = x + (x >> 8);
    x = x + (x >> 16);
    return x & 0x3Fu;   /* max 32 bits => result fits in 6 bits */
}