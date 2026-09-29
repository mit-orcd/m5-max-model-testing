int cmp_desc(const void *a, const void *b) {
    const int *x = a, *y = b;
    int d = *x - *y;
    return (d > 0) - (d < 0);
}