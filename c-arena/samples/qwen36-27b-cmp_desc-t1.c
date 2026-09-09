int cmp_desc(const void *a, const void *b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    
    if (*ia > *ib) return 1;
    if (*ia < *ib) return -1;
    return 0;
}