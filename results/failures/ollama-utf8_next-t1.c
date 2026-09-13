```c
#include <stdint.h>
#include <stddef.h>

int utf8_next(const unsigned char *s, size_t n, uint32_t *cp)
{
    if (n < 1)
        return -1;

    unsigned char c0 = s[0];

    /* 1-byte: 0xxxxxxx */
    if (c0 < 0x80) {
        *cp = (uint32_t)c0;
        return 1;
    }

    /* Reject continuation byte (10xxxxxx) in leading position */
    if ((c0 & 0xC0) == 0x80)
        return -1;

    /* 2-byte: 110xxxxx 10xxxxxx */
    if ((c0 & 0xE0) == 0xC0) {
        if (n < 2)
            return -1;
        unsigned char c1 = s[1];
        if ((c1 & 0xC0) != 0x80)
            return -1;

        uint32_t code = ((uint32_t)(c0 & 0x1F) <<