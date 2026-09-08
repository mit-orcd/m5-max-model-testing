int cmp_desc(const void *a, const void *b) {
    int ia = *(int*)a;
    int ib = *(int*)b;
    return ia < ib ? -1 : ia > ib;
}