int cmp_desc(const void *a, const void *b) {
    int ia = *(int*)a;
    int ib = *(int*)b;
    if (ia == ib) return 0;
    if (ia < ib) return -1;
    return 1;
}