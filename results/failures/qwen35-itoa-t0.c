```c
#include <limits.h>

void itoa(int value, char *buf) {
    int i = 0;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        // INT_MIN is -2147483648, so we can't do -value
        // We will construct the string manually or use a safe negation strategy
        // Strategy: convert to negative string logic or handle digits directly
        // Since INT_MIN is the only value that cannot be negated to positive int,
        // we handle it by converting to negative digits.
        
        // Special case for INT_MIN: "-2147483648"
        // We can't do -INT_MIN, so we work with negative numbers throughout.
        // But standard logic usually converts to positive.
        // Let's use a buffer to store digits in reverse, then reverse the whole string.
        
        // Actually, simpler approach: 
        // If value is INT_MIN, we can't negate it.
        // We can just handle the digits manually or use a different approach.
        // Let's use the standard approach but handle the negation carefully.
        
        // Alternative: Use a temporary buffer for digits, then reverse.
        // But we need to handle the sign.
        
        // Let's rewrite the logic to handle negative numbers by keeping them negative
        // and extracting digits using modulo, but modulo on negative numbers in C
        // is implementation-defined (usually truncates towards zero).
        // C11 defines truncation towards zero.
        // -123 % 10 = -3.
        // So we can work with negative numbers directly.
        
        // However, INT_MIN % 10 is -8.
        // -INT_MIN is undefined behavior (overflow).
        // So we must NOT negate INT_MIN.
        
        // Strategy:
        // If value is INT_MIN, we treat it as a special case or work with negative numbers.
        // Let's work with negative numbers for all inputs <= 0.
        
        // If value is 0, handle separately.
        if (value == 0) {
            buf[0] = '0';
            buf[1] = '\0';
            return;
        }

        if (value < 0) {
            is_negative = 1;
            // We will work with negative numbers to avoid overflow on INT_MIN
            // But we need to be careful with the loop condition and modulo.
            // In C11, -123 % 10 is -3.
            // -123 / 10 is -12.
            // This works fine for generating digits.
        }

        // Extract digits
        // We need to store them in reverse order.
        // Since we don't know the length, we can use a temporary buffer or calculate length.
        // Or just write backwards from the end of buf.
        
        // Let's find the length first? No, we can write backwards.
        // But we need to know where to start.
        // Let's use a temporary buffer for digits.
        char temp[12]; // Max 10 digits + sign + null
        int t_idx = 0;
        
        int temp_val = value;
        
        // If value is INT_MIN, temp_val is INT_MIN.
        // We can't negate it.
        // So we process it as negative.
        
        while (temp_val != 0) {
            int digit = temp_val % 10;
            temp_val = temp_val / 10;
            
            // digit is negative (e.g., -3)
            // Convert to char: '0' + abs(digit)
            // Since digit is negative, we can do '0' - digit?
            // -3 -> '0' - (-3) = '3'. Correct.
            // But wait, if digit is -3, '0' - (-3) = 48 + 3 = 51 ('3').
            // Yes.
            temp[t_idx++] = '0' - digit;
        }
        
        if (is_negative) {
            buf[i++] = '-';
        }
        
        // Copy reversed digits
        for (int k = 0; k < t_idx; k++) {
            buf[i++] = temp[k];
        }
        
        buf[i] = '\0';
        return;
    }

    // Positive numbers
    if (value == 0)