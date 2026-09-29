#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        // INT_MIN cannot be negated to positive int
        // Handle as string "-2147483648"
        *p++ = '-';
        // Subtract 1 to make it easier to handle as positive (becomes 2147483647)
        // Then process 2147483647 and append '0' at the end? No, easier to just use division.
        // Actually, let's just handle INT_MIN by dividing by -10.
        // But division of INT_MIN by -10 is safe.
        // Wait, standard approach:
        // If negative, make positive. But INT_MIN is problematic.
        // Strategy: Process digits from the end or use a temporary buffer.
        // Simpler: Use a local buffer to reverse.
        char temp[12]; // Enough for "-2147483648" + null
        char *tp = temp;
        int v = value;
        
        if (v == INT_MIN) {
            *tp++ = '-';
            // Convert -2147483648 to digits.
            // Since we can't negate, we can do:
            // -2147483648 % 10 = -8
            // -2147483648 / 10 = -214748364
            // This works in C11 (truncation towards zero).
            // We collect negative digits and negate them later.
            do {
                int digit = v % 10;
                *tp++ = (digit < 0) ? -digit + '0' : digit + '0';
                v /= 10;
            } while (v != 0);
        } else {
            if (v < 0) {
                *tp++ = '-';
                v = -v;
            }
            do {
                int digit = v % 10;
                *tp++ = digit + '0';
                v /= 10;
            } while (v != 0);
        }
        *tp = '\0';
        
        // Reverse the buffer
        char *start = temp;
        char *end = tp - 1;
        while (start < end) {
            char t = *start;
            *start++ = *end;
            *end-- = t;
        }
        
        // Copy to output
        while (*temp) {
            *p++ = *temp++;
        }
        *p = '\0';
        return;
    }

    if (value < 0) {
        *p++ = '-';
        value = -value;
    } else if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    // Collect digits in reverse order
    char temp[12];
    char *tp = temp;
    while (value > 0) {
        int digit = value % 10;
        *tp++ = digit + '0';
        value /= 10;
    }

    // Reverse into buf
    while (tp > temp) {
        *p++ = *--tp;
    }
    *p = '\0';
}