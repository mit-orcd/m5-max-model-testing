#include <stddef.h>

typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block;

static char *arena_buf;
static size_t arena_size;
static block *head;

void arena_init(void *buf, size_t size) {
    arena_buf = (char *)buf;
    arena_size = size;

    size_t hdr = (sizeof(block) + 15u) & ~(size_t)15;
    if (size < hdr) {
        head = NULL;
        return;
    }
    block *b = (block *)arena_buf;
    b->size = size - hdr;
    b->free = 1;
    b->next = NULL;
    head = b;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    size_t need = n + (sizeof(block) + 15u) & ~(size_t)15;

    block *prev = NULL;
    block *b = head;
    while (b) {
        if (b->free && b->size >= need) break;
        prev = b;
        b = b->next;
    }

    if (!b) return NULL;

    size_t remaining = b->size - need;
    if (remaining > sizeof(block)) {
        block *split = (block *)((char *)b + hdr + need);
        split->size = remaining - hdr;
        split->free = 1;
        split->next = b->next;
        if (prev) prev->next = split;
        else head = split;
        b->size = need;
    } else {
        b->size = 0;
        if (prev) prev->next = b->next;
        else head = b->next;
    }

    return (char *)b + hdr;
}

void arena_free(void *p) {
    if (!p) return;
    size_t hdr = (sizeof(block) + 15u) & ~(size_t)15;
    block *b = (block *)((char *)p - hdr);
    b->free = 1;

    if (prev && prev->free) {
        prev->size += sizeof(block) + b->size;
        prev->next = b->next;
        b = prev;
    }
    if (b->next && b->next->free) {
        b->size += sizeof(block) + b->next->size;
        b->next = b->next->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    size_t hdr = (sizeof(block) + 15u) & ~(size_t)15;
    block *b = (block *)((char *)p - hdr);
    size_t old = b->size;

    if (old >= n) return p;

    block *after = b->next;
    if (after && after->free) {
        size_t can = b->size + sizeof(block) + after->size;
        if (can >= n) {
            b->size = can;
            b->next = after->next;
            return p;
        }
    }

    void *np = arena_alloc(n);
    if (!np) return NULL;
    size_t cp = old < n ? old : n;
    __builtin_memcpy(np, p, cp);
    arena_free(p);
    return np;
}