```c
#include <limits.h>
#include <stddef.h>

void itoa(int value, char *buf) {
    char *p = buf;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN specially to avoid overflow when negating
        // INT_MIN = -2147483648
        // We can't do -value, so we subtract 1 and negate, then add 1 at the end
        // Or simpler: manually handle the digits.
        // Let's use a safe approach: subtract 1, negate, then add 1 to the last digit.
        // Actually, simpler: just process digits by subtracting 10 repeatedly or using modulo.
        // But modulo on negative numbers in C is implementation-defined before C99, 
        // and in C99/C11 it truncates towards zero.
        // -2147483648 % 10 = -8.
        // So we can use modulo and handle the sign.
        
        // Let's stick to the standard logic:
        // If value is INT_MIN, we can't do -value.
        // We can do: value = value + 1; is_negative = 1; then process, then add 1 to the last digit.
        // But that's messy.
        
        // Better approach:
        // Use a temporary buffer or just handle the digits carefully.
        // Since we are writing to buf, we can just do:
        // if (value == INT_MIN) {
        //     // -2147483648
        //     // We can't negate it.
        //     // Let's just subtract 10 repeatedly? No, too slow.
        //     // Let's use the property that -2147483648 % 10 = -8.
        //     // We can collect digits as negative numbers and then convert to positive chars.
        // }
        
        // Standard robust way:
        // If value is INT_MIN, we can't negate it.
        // We can do: value = value + 1; is_negative = 1; then process, then add 1 to the last digit.
        // But let's try a different way:
        // Just use the fact that we can handle negative numbers by taking modulo.
        // In C11, -2147483648 % 10 is -8.
        // So we can do:
        // while (value != 0) {
        //     int digit = value % 10;
        //     value /= 10;
        //     *p++ = '0' + (digit < 0 ? -digit : digit);
        // }
        // But wait, -2147483648 / 10 is -214748364.
        // -214748364 % 10 is -4.
        // This works!
        // The only issue is if value is 0, we need to handle it.
        
        // Let's implement the general case:
        // If value is INT_MIN, we can't do -value, but we can do value % 10 and value / 10.
        // The only problem is if value is 0, we need to output "0".
        
        // So:
        // if (value == 0) {
        //     *p++ = '0';
        //     *p = '\0';
        //     return;
        // }
        // if (value < 0) {
        //     is_negative = 1;
        //     // We can't do -value if value is INT_MIN.
        //     // But we can process digits as negative.
        // }
        
        // Let's rewrite the loop to handle negative digits correctly.
        // We will collect digits in reverse order.
        // If value is INT_MIN, we can't negate it, but we can still do modulo and division.
        // The digits will be negative.
        // We can convert them to positive by taking absolute value.
        // But abs(INT_MIN) is undefined behavior.
        // So we must avoid abs(INT_MIN).
        // Instead, we can do: digit = value % 10; value /= 10;
        // Then digit is negative.
        // We can convert digit to positive by: digit = -digit;
        // But if digit is -