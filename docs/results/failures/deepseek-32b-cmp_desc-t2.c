int cmp_desc(const void *a, const void *b) {
    int a_val = *(const int *)a;
    int b_val = *(const int *)b;
    if (a_val > b_val) return 1;
    else if (a_val < b_val) return -1;
    else return 0;
}