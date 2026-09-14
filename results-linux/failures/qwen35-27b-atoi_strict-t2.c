#include <stddef.h>
#include <ctype.h>

int atoi_strict(const char *s, int *out) {
    if (s == NULL || out == NULL) {
        return -1;
    }

    const char *p = s;

    /* Skip leading whitespace if any (though prompt implies strict, usually atoi allows leading whitespace)
     * However, "exactly one valid integer" with "trailing chars" check suggests strictness.
     * Standard atoi allows leading whitespace. Let's assume standard behavior for leading whitespace
     * but strictness for trailing non-whitespace or content.
     * Re-reading: "Parses... from s". "Returns -1 if s is not exactly one valid integer (empty, junk, trailing chars)".
     * Usually "trailing chars" implies no garbage after the number.
     * Let's handle optional leading whitespace as per standard integer parsing, then the number, then ensure end.
     */
    
    /* Skip leading whitespace */
    while (isspace((unsigned char)*p)) {
        p++;
    }

    if (*p == '\0') {
        return -1; /* Empty or only whitespace */
    }

    int sign = 1;
    if (*p == '-') {
        sign = -1;
        p++;
    } else if (*p == '+') {
        p++;
    }

    if (*p == '\0' || !isdigit((unsigned char)*p)) {
        return -1; /* No digits found after sign or start */
    }

    /* Parse digits */
    long long val = 0; /* Use long long to hold intermediate, though no overflow test required */
    while (isdigit((unsigned char)*p)) {
        val = val * 10 + (*p - '0');
        p++;
    }

    /* Check for trailing characters */
    if (*p != '\0') {
        return -1;
    }

    /* Convert to int (no overflow check required per spec) */
    *out = (int)(val * sign);

    return 0;
}