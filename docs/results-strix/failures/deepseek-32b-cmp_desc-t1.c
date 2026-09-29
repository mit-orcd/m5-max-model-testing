int cmp_desc(const void *a, const void *b) {
    int val_a = *(int *)a;
    int val_b = *(int *)b;
    if (val_a > val_b) return 1;
    if (val_a < val_b) return -1;
    return 0;
}