
#include <stdio.h>
#include <stdint.h>
uint32_t bswap32(uint32_t x);
static int fails = 0;
static void check(uint32_t x, uint32_t want) {
    uint32_t got = bswap32(x);
    if (got != want) { printf("FAIL bswap32(0x%08X)=0x%08X want 0x%08X\n", x, got, want); fails++; }
}
int main(void) {
    check(0x12345678u, 0x78563412u); check(0x00000001u, 0x01000000u);
    check(0u, 0u); check(0xAABBCCDDu, 0xDDCCBBAAu); check(0xFF0000FFu, 0xFF0000FFu);
    if (!fails) printf("PASS\n");
    return fails ? 1 : 0;
}

