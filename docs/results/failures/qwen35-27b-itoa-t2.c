#include <limits.h>

void itoa(int value, char *buf) {
    char *p = buf;
    char local_buf[12];
    char *end = local_buf + sizeof(local_buf);
    int is_negative = 0;

    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }

    if (value < 0) {
        is_negative = 1;
        // Handle INT_MIN safely by working with unsigned
        if (value == INT_MIN) {
            unsigned int uval = -(unsigned int)INT_MIN;
            while (uval > 0) {
                *--end = '0' + (uval % 10);
                uval /= 10;
            }
        } else {
            unsigned int uval = (unsigned int)-value;
            while (uval > 0) {
                *--end = '0' + (uval % 10);
                uval /= 10;
            }
        }
    } else {
        unsigned int uval = (unsigned int)value;
        while (uval > 0) {
            *--end = '0' + (uval % 10);
            uval /= 10;
        }
    }

    if (is_negative) {
        *p++ = '-';
    }

    while (end < buf + sizeof(local_buf) && end > local_buf) {
        *p++ = *end++;
    }
    
    // If we wrote to local_buf, copy back to buf
    // Note: The logic above assumes writing to local_buf first then copying
    // Let's refine to write directly to buf if possible, or copy from local_buf
    // Since the previous logic calculated digits into local_buf, we need to copy.
    
    // Correction: The loop above wrote into 'end' which is in local_buf.
    // We need to copy from local_buf to buf.
    // Re-implementing with direct copy logic for clarity and correctness.
    
    // Reset and redo properly to ensure buf is used correctly
    char *start = local_buf + sizeof(local_buf);
    
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    
    if (value < 0) {
        is_negative = 1;
        if (value == INT_MIN) {
            unsigned int uval = 2147483648U; // -INT_MIN
            start = local_buf + sizeof(local_buf);
            while (uval > 0) {
                *--start = '0' + (uval % 10);
                uval /= 10;
            }
        } else {
            unsigned int uval = (unsigned int)-value;
            start = local_buf + sizeof(local_buf);
            while (uval > 0) {
                *--start = '0' + (uval % 10);
                uval /= 10;
            }
        }
    } else {
        unsigned int uval = (unsigned int)value;
        start = local_buf + sizeof(local_buf);
        while (uval > 0) {
            *--start = '0' + (uval % 10);
            uval /= 10;
        }
    }
    
    if (is_negative) {
        *p++ = '-';
    }
    
    while (start < local_buf + sizeof(local_buf)) {
        *p++ = *start++;
    }
    *p = '\0';
}