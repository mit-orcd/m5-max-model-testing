int cmp_desc(const void *a, const void *b) {
    int x = *(int*)a;
    int y = *(int*)b;
    return y - x;
}