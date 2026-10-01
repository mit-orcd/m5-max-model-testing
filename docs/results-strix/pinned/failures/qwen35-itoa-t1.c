#include <limits.h>

void itoa(int value, char *buf) {
    int i = 0;
    int is_negative = 0;

    if (value == INT_MIN) {
        // Handle INT_MIN separately to avoid overflow when negating
        buf[i++] = '-';
        value = -(value + 1); // value becomes -(INT_MIN + 1) = INT_MAX
        // We will print INT_MAX + 1 digits, then add '1' at the end
        // Actually, simpler approach: convert -(INT_MIN + 1) which is INT_MAX,
        // then append '1' at the end.
        // Let's do standard conversion for the positive part of -(INT_MIN + 1)
        // which is INT_MAX.
        // Wait, -(INT_MIN) is undefined. -(INT_MIN + 1) is INT_MAX.
        // So we print INT_MAX, then append '1'.
        // But standard logic:
        // value = -(value + 1); // value is now INT_MAX
        // convert value to string
        // append '1'
        // But this is tricky.
        // Better: use a temporary buffer or handle digits manually.
        // Let's use the standard approach with a temporary buffer for digits.
    } else if (value < 0) {
        buf[i++] = '-';
        value = -value;
    }

    if (value == 0) {
        buf[i++] = '0';
    } else {
        char temp[12]; // Enough for INT_MIN
        int j = 0;
        int temp_val = value;
        
        // If value was INT_MIN, we handled it above? No, let's restart logic cleanly.
    }
}