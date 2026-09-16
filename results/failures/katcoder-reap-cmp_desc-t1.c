int cmp_desc(const void *a, const void *b) {
    int x = (int)a;
    int y = (int)b;
    if (x > y) return -1;
    if (x < y) return 1;
    return 0;
}