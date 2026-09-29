int cmp_desc(const void *a, const void *b) {
    const int *pa = a, *pb = b;
    if (*pa == *pb) return 0;
    return (*pa > *pb) - (*pa < *pb);
}