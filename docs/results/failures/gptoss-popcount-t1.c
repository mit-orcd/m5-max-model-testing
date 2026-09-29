/* Count the number of set bits (population count) in an unsigned integer. */
static inline unsigned popcount(unsigned x)
{
    /* SWAR (SIMD Within A Register) algorithm – works on any 32‑bit unsigned int. */
    x -= (x >> 1) & 0x55555555u;          /* put count of each 2 bits into each 2‑bit field */
    x = (x & 0x33333333u) + ((x >> 2) & 0x33333333u);  /* sum pairs of 2‑bit fields into 4‑bit fields */
    x = (x + (x >> 4)) & 0x0F0F0F0Fu;     /* sum 4‑bit fields into 8‑bit fields */
    return (x * 0x01010101u) >> 24;       /* sum the 8‑bit fields into the high byte */
}