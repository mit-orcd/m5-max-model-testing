int cmp_desc(const void *a, const void *b) {
    int val_a = *(const int *)a;
    int val_b = *(const int *)b;

    if (val_a > val_b) return 1;
    if (val_a < val_b) return -1;
    return 0;
}