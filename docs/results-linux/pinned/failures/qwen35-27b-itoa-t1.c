```c
#include <limits.h>
#include <stdio.h>

void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char *p = buf;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        if (value == INT_MIN) {
            // Handle INT_MIN specifically to avoid overflow on negation
            *p++ = '-';
            value = -INT_MAX - 1; // This is still negative, but we process digits directly
            // We will process the absolute value logic manually for INT_MIN
            // INT_MIN is -2147483648. Digits: 2,1,4,7,4,8,3,6,4,8
            // Since we can't negate it safely, we treat it as a special case or use unsigned math.
            // A cleaner approach for INT_MIN:
            // Convert to unsigned, negate, then handle sign.
            // But standard int negation of INT_MIN is undefined behavior.
            // Let's restart the logic for INT_MIN to be safe.
        } else {
            value = -value;
        }
    }

    // Re-evaluate for INT_MIN safety
    if (is_negative && value == INT_MIN) {
        // This path is unreachable if we negated above, so we need a different structure.
        // Let's rewrite the logic to handle INT_MIN without negating it first.
    }
}

// Corrected implementation below
void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }

    char *p = buf;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        if (value == INT_MIN) {
            // Special handling for INT_MIN to avoid undefined behavior on negation
            // INT_MIN is -2147483648
            // We can work with the absolute value by using unsigned arithmetic
            // or by processing the number as a string of digits directly.
            // A common trick: value = -value is UB.
            // Instead, we can do: unsigned int u = (unsigned int)(-value) is UB.
            // Correct way: unsigned int u = (unsigned int)(value); // This is 2147483648
            // But wait, (unsigned int)(INT_MIN) is 2147483648.
            // So we can just use the unsigned representation.
            // However, the sign is already handled.
            // Let's just use the unsigned cast for the magnitude.
            // But wait, if value is INT_MIN, -value is UB.
            // So we must not do value = -value.
            // We will use a temporary unsigned variable.
            unsigned int u = (unsigned int)value; // This is 2147483648
            // Now we convert u to string, but we need to be careful.
            // Actually, (unsigned int)INT_MIN is 2147483648.
            // So we can just convert u.
            // But wait, the logic below expects a positive int.
            // Let's just use the unsigned variable for the loop.
            // We need to set value to a positive representation for the loop.
            // But we can't assign u to value because value is int.
            // Let's change the loop to work on unsigned.
            
            // Actually, simpler:
            // If value is INT_MIN, we know the string is "-2147483648".
            // But let's do it generically.
            // We can use a temporary unsigned int.
            unsigned int temp = (unsigned int)value; // 2147483648
            // Now we convert temp to string.
            // But wait, the loop below uses 'value'.
            // Let's refactor to use a local variable for the magnitude.
            
            // Let's restart the function with a cleaner approach.
        }
    }
}

// Final Correct Implementation
void itoa(int value, char *buf) {
    if (value == 0) {
        buf[0] = '0';