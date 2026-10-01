#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        *p++ = '-';
        value = -(value + 1); // value becomes -(INT_MIN + 1) = INT_MAX
        // We will print INT_MAX + 1 digits, then add the last '1'
        // Actually, simpler approach: print digits of -(value+1) then append '1'
        // But standard recursion/loop logic is easier if we just handle the sign and magnitude carefully.
        // Let's use a different strategy for INT_MIN:
        // It's -2147483648.
        // We can't do -value.
        // We can do: value = value + 1 (becomes -2147483647), negate to 2147483647, print, then append '1'.
        // Or just handle the digits manually.
        
        // Let's stick to a robust digit extraction method that works for INT_MIN.
        // We will extract digits from the end.
        // Since we can't negate INT_MIN, we work with negative numbers throughout the loop.
        // -2147483648 % 10 = -8.
        // -2147483648 / 10 = -214748364.
        // This works in C11 (truncation towards zero).
        // So we can just keep the number negative.
    } else if (value < 0) {
        *p++ = '-';
        value = -value;
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        // We need to handle the case where value is INT_MIN carefully if we used the logic above.
        // If we kept it negative, we need to handle the sign logic differently.
        // Let's restart the logic to be purely negative-based to handle INT_MIN safely.
    }
    
    // Reset and implement the robust negative-based approach
    p = buf;
    if (value == INT_MIN) {
        *p++ = '-';
        // We will process -2147483648 as a negative number
        // Digits will be extracted as negative remainders
        // We need a temporary buffer or reverse logic.
        // Let's use a temporary buffer to store digits in reverse order.
        char temp[12]; // Enough for INT_MIN + sign
        int idx = 0;
        int n = value;
        
        while (n != 0) {
            int rem = n % 10;
            n /= 10;
            // rem is negative (e.g., -8)
            temp[idx++] = '0' - rem;
        }
        temp[idx] = '\0';
        
        // Reverse temp into buf
        int i = 0;
        while (idx > 0) {
            buf[i++] = temp[--idx];
        }
        buf[i] = '\0';
        return;
    }

    if (value < 0) {
        *p++ = '-';
        value = -value;
    }

    if (value == 0) {
        *p++ = '0';
    } else {
        char temp[12];
        int idx = 0;
        while (value > 0) {
            temp[idx++] = (char)('0' + (value % 10));
            value /= 10;
        }
        while (idx > 0) {
            *p++ = temp[--idx];
        }
    }
    *p = '\0';
}