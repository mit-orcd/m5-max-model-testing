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
            // A robust way: convert to unsigned, but handle the sign separately.
            // Let's restart the logic for INT_MIN to be cleaner.
            // Actually, simpler: use unsigned arithmetic for the magnitude.
            unsigned int u_val = (unsigned int)INT_MIN; 
            // But wait, (unsigned int)INT_MIN is 2147483648.
            // We need to print "2147483648".
            // Let's just use a generic loop with unsigned conversion.
        } else {
            value = -value;
        }
    }

    // Re-evaluating for a cleaner, unified approach without special casing INT_MIN in the flow
    // Reset pointer
    p = buf;
    
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0) {
        *p++ = '-';
        // Convert to unsigned to handle INT_MIN safely
        // Note: (unsigned int)INT_MIN is the correct magnitude for printing
        unsigned int u_val = (unsigned int)value;
        // However, if value was not INT_MIN, we negated it above? No, let's do it cleanly.
        // Let's restart the function logic for maximum clarity and correctness.
    }
}

// Corrected implementation below
void itoa(int value, char *buf) {
    char *p = buf;
    
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0) {
        *p++ = '-';
        // Cast to unsigned to handle INT_MIN without overflow
        // INT_MIN is -2147483648. (unsigned int)INT_MIN is 2147483648.
        // This is the correct magnitude to print.
        unsigned int u_val = (unsigned int)value;
        
        // Buffer for digits (max 10 digits + null)
        char temp[12];
        char *tp = temp;
        
        while (u_val > 0) {
            *tp++ = (u_val % 10) + '0';
            u_val /= 10;
        }
        
        // Copy reversed digits to buffer
        while (tp > temp) {
            *p++ = *--tp;
        }
    } else {
        unsigned int u_val = (unsigned int)value;
        char temp[12];
        char *tp = temp;
        
        while (u_val > 0) {
            *tp++ = (u_val % 10) + '0';
            u_val /= 10;
        }
        
        while (tp > temp) {
            *p++ = *--tp;
        }
    }
    
    *p = '\0';
}