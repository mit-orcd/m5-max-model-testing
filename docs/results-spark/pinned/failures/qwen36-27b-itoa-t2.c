```c
#include <limits.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char temp[12]; // Enough for INT_MIN (-2147483648) + null terminator
    int i = 0;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        // Handle INT_MIN specially to avoid overflow when negating
        if (value == INT_MIN) {
            // INT_MIN is -2147483648
            // We can process it by taking absolute value in parts or using unsigned
            // A safe way: convert using unsigned arithmetic
            unsigned int uval = (unsigned int)(-(value + 1)) + 1; // This is tricky, let's use a different approach
            // Actually, simpler: just use unsigned conversion for the magnitude
            // But value is int. Let's cast to unsigned after handling sign.
            // For INT_MIN, -value overflows. So we handle it by converting the positive part.
            // INT_MIN = -2147483648. Magnitude is 2147483648.
            // We can write the digits directly or use unsigned long.
            // Let's use a generic approach with unsigned long for the magnitude.
            unsigned long mag = (unsigned long)(-(long)value);
            // But -(long)INT_MIN might still be problematic if long is 32-bit.
            // On most systems long is 64-bit, but to be strictly portable for 32-bit int:
            // Let's just do digit extraction carefully.
            
            // Alternative: process the number as unsigned by casting after negation? No.
            // Best approach for INT_MIN: 
            // We know INT_MIN is -2147483648.
            // Let's just use a loop that works for negative numbers by dividing by 10.
            // In C11, division of negative numbers truncates towards zero.
            // -123 / 10 = -12, -123 % 10 = -3.
            // So we can work with negative numbers directly.
            
            // Reset and use negative arithmetic
            i = 0;
            int n = value;
            while (n != 0) {
                int digit = n % 10;
                n /= 10;
                // digit is negative or zero
                temp[i++] = (char)('0' - digit);
            }
        } else {
            // General negative case
            int n = value;
            while (n != 0) {
                int digit = n % 10;
                n /= 10;
                temp[i++] = (char)('0' - digit);
            }
        }
    } else {
        // Positive case
        int n = value;
        while (n != 0) {
            int digit = n % 10;
            n /= 10;
            temp[i++] = (char)('0' + digit);
        }
    }

    // If we handled INT_MIN or other negatives in the special block above, we need to ensure consistency.
    // The above logic for negative numbers (including INT_MIN) uses the 'n' variable and extracts digits.
    // However, the INT_MIN block and the else block are duplicated. Let's refactor.
    
    // Actually, the logic `n % 10` for negative `n` yields negative remainder.
    // e.g. -123 % 10 = -3. '0' - (-3) = '3'. Correct.
    // -10 % 10 = 0. '0' - 0 = '0'. Correct.
    // So the same loop works for all negative numbers, including INT_MIN, provided we don't negate INT_MIN.
    
    // Let's rewrite cleanly:
    
    // Reset
    i = 0;
    int n = value;
    
    if (n == 0) {
        temp[i++] = '0';
    } else if (n < 0) {
        while (n != 0) {
            int digit = n % 10;
            n /= 10;
            temp[i++] = (char)('0' - digit);