int cmp_desc(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    /* Overflow-safe: no subtraction. */
    return (x < y) - (x > y);
}
