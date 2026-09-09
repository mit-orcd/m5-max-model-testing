```c
#include <stddef.h>
#include <limits.h>

/* Extract the idx-th (0‑based) field from an RFC‑4180 CSV line.
 * The field is unescaped and NUL‑terminated into out, truncated to cap‑1 chars.
 * Return the unescaped length, or -1 if idx is out of range. */
int csv_field(const char *line, int idx, char *out, size_t cap)
{
    const char *p = line;
    int field = 0;
    /* Determine if the line ends with a trailing comma (empty field). */
    const char *end = line;
    while (*end && *end != '\r' && *end != '\n')
        ++end;
    int trailing_comma = (end > line && *(end - 1) == ',');

    /* Skip fields up to idx. */
    while (field < idx) {
        /* If we reach the end before skipping enough fields, idx is out of range. */
        if (*p == '\0' || *p == '\r' || *p == '\n')
            return -1;

        /* Skip one field