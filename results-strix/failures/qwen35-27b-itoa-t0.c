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
            // Let's use unsigned math for the conversion loop.
            unsigned int u_val = (unsigned int)INT_MIN; 
            // Actually, simpler: just use unsigned int for the loop logic.
            // But we already printed '-'. Now we need to print 2147483648.
            // Let's restart the logic to be cleaner.
        }
    }

    // Cleaner approach: use unsigned int for the magnitude
    unsigned int u_val;
    if (is_negative) {
        if (value == INT_MIN) {
            *p++ = '-';
            u_val = (unsigned int)INT_MIN; // This is 2147483648
        } else {
            *p++ = '-';
            u_val = (unsigned int)(-value);
        }
    } else {
        u_val = (unsigned int)value;
    }

    char *end = p;
    while (u_val > 0) {
        *end++ = (u_val % 10) + '0';
        u_val /= 10;
    }
    *end = '\0';

    // Reverse the string in place
    char *start = p;
    while (start < end - 1) {
        char temp = *start;
        *start = *(end - 1);
        *(end - 1) = temp;
        start++;
        end--;
    }
}