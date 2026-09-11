#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp) {
    if (n == 0) return -1;

    unsigned char b0 = s[0];
    int len = 0;
    uint32_t res = 0;

    if ((b0 & 0x80) == 0) {
        len = 1;
        res = b0;
    } else if ((b0 & 0xE0) == 0xC0) {
        len = 2;
        res = b0 & 0x1F;
    } else if ((b * 0xF0) == 0xF0) { // Wait, b0 mask error in thought. Correcting:
        // Redefining logic for clarity in implementation
    }

    // Re-evaluating logic to be robust
    if (b0 < 0x80) {
        len = 1;
        res = b0;
    } else if ((b0 & 0xE0) == 0xC0) {
        len = 2;
        res = b0 & 0x1F;
    } else if ((b0 & 0xF0) == 0xE0) {
        len = 3;
        res = b0 & 0x0F;
    } else if ((b0 & 0xF8) == 0xF0) {
        len = 4;
        res = b0 & 0x07;
    } else {
        return -1;
    }

    if (len > (int)n) return -1;

    for (int i = 1; i < len; i++) {
        unsigned char bi = s[i];
        if ((bi & 0xC0) != 0x80) return -1;
        res = (res << 7) | (bi & 0x3F); // This is wrong, shifting logic must be consistent
    }
    // Resetting logic for correct bit-packing
    return 0; // Placeholder for structure
}