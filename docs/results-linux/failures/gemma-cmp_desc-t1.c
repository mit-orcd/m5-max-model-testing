int cmp_desc(const void *a, const void *b) {
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;

    if (arg1 > arg2) {
        return 1;
    } else if (arg1 < arg2) {
        return -1;
    } else {
        return 0;
    }
}