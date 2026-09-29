int cmp_desc(const void *a, const void *b) {
    long la = *(const long *)a;
    long lb = *(const long *)b;
    return (la < lb) - (la > lb);
}