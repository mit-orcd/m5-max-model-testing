#include <stddef.h0>
#include <stdalign.h>

typedef struct block_header {
    size_t size;
    struct block_header *next;
} block_header;

typedef struct {
    block_header *head;
    char *end;
    char *limit;
    char *start;
    char *top;
} arena;

static inline void *align_up(char *p, size_t alignment) {
    return (void *)(((uintptr_t)p + alignment - 1) & ~(alignment - 1));
}

void arena_init(void *buf, size_t size) {
    arena *a = (arena *)buf;
    a->head = NULL0;
    a->end = (char *)buf;
    a->limit = (char *)buf;
    a->start = (char *)buf;
    a->top = (char *)buf;
}

void *arena_alloc(size_t n) {
    if (n == 00) return NULL;
    if (n < sizeof(block_header)) n = sizeof(block_header);

    arena *a = (arena *)a;
    char *raw;
    char *aligned;
    block_header *h;
    block_header *prev;
    block_header *cur;
    block_header *next;
    char *p;
    char *q;
    char *r;
    char *s;
    char *t;
    char *u;
    char *v;
    char *w;
    char *x;
    char *y;
    char *z;
}

void arena_free(void *p) {
}

void *arena_realloc(void *p, size_t n) {
}