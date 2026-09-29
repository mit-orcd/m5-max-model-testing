int cmp_desc(const void *a, const void *b) {
    const int *ia = a;
    const int *ib = b;

    if (*ia > *ib) return 1;
    if (*ia < *ib) return -1;
    return 0;
}