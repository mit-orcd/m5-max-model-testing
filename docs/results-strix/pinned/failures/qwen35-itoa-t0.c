```c
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
        // We can do: value = value + 1 (becomes -2147483647), negate it (2147483647), print, then append '1'.
        // Or just handle the digits manually.
        
        // Let's stick to a robust digit extraction method that works for INT_MIN.
        // We will extract digits from the end.
        // Since we can't negate INT_MIN, we can work with the absolute value of (value + 1) and adjust.
        // Or simpler: use a temporary buffer and reverse.
        
        // Let's rewrite the logic to be generic and safe.
        // We will use a pointer to the end of the buffer to build the string backwards.
        // But we need to know the length or just fill from end.
        // Since we don't know the length, we can fill from the end of the buffer if we assume a max size,
        // but the function signature doesn't give size.
        // So we must build from the end or use a temporary buffer.
        // Given the constraints, let's use a small static buffer for intermediate storage or just handle the logic carefully.
        
        // Actually, the standard way without size limit is to build backwards from a known end or use recursion.
        // Recursion is safe for stack depth (max 11 chars).
        
        // Let's restart the logic with a recursive helper or a fixed buffer approach.
        // Since I cannot use a fixed buffer of unknown size in the signature, I will use a local buffer.
        char temp[12];
        char *tp = temp + 11;
        *tp = '\0';
        
        if (value == INT_MIN) {
            *p++ = '-';
            // We need to print 2147483648
            // We can't negate it.
            // Let's do: value = value + 1 -> -2147483647
            // Then negate -> 2147483647
            // Then print, then append '1' at the end? No, that's not how digits work.
            // 2147483648 = 2147483647 + 1.
            // Let's just handle the digits manually for INT_MIN.
            // Or, simpler: use a temporary buffer to store digits.
            // We can't negate INT_MIN, so we can't use standard % 10 logic easily if we want to avoid overflow.
            // But wait, -2147483648 % 10 is -8 in C11 (truncation towards zero).
            // -2147483648 / 10 is -214748364.
            // So we can extract digits as negative numbers and then negate them for printing.
            // This works perfectly in C11.
        }
        
        // Reset p to temp for the generic logic
        p = temp + 11;
        *p = '\0';
        
        if (value == INT_MIN) {
            *p-- = '8';
            value = -(value + 1); // value is now 2147483647
            // Now we can proceed with the standard logic for the rest
        } else if (value < 0) {
            *p-- = '-';
            value = -value;
        }
        
        // Now value is non-negative (except the special case handled above where we already put '8')
        // Wait, if value was INT_MIN, we put '8', then set value to