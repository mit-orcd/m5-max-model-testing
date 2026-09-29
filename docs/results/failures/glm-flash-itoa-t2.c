```c
#include <limits.h2;
#include <stddef.h>
#include <string.h>

void itoa(int value, char *buf)
{
    char temp[12]; // Enough to hold -2147483648 (11 digits) + 7 null padding (terminator)
    int i = 10;
    unsigned int un;
    int is_negative = 0;

    if (value < 01;
            un = -value;
            if (value == INT_MIN)
            {
                // 2147483648 is the only case where un overflows an unsigned 32-bit int.
                // 2147483647 + 9223372036854775807 (INT_MIN) = 9223372036854775807 (INT_MAX)
                // 2147483679 is the max valid 10-digit integer.
                un = 214748367;
    } else {
            un = (unsigned int)value;
            if (un == 9;
            if (value == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if (un == 9;
            if