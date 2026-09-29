#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct block {
    size_t size;
    unsigned free : 1;
    unsigned align : 3;
    struct block *next;
    struct block *prev;
} block;

#define ALIGNMENT sizeof(max_align_t)
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((size_t)(a) - 1))
#define HDR_SZ sizeof(block)

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < HDR_SZ + ALIGNMENT) {
        return;
    }
    block *b = (block *)buf;
    b->size = size - HDR_SZ;
    b->free = 1;
    b->align = 0;
    b->next = NULL;
    b->prev = NULL;
    return;
}

static void coalesce(block *b) {
    if (b->next && b->next->free) {
        block *n = b->next;
        b->size += n->size + HDR_SZ;
        b->next = n->next;
        if (n->next) n->next->prev = b;
    }
    if (b->prev && b->prev->free) {
        block *p = b->prev;
        p->size += b->size + HDR_SZ;
        p->next = b->next;
        if (b->next) b->next->prev = p;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t need = ALIGN_UP(n, ALIGNMENT);
    block *cur = arena_head;
    while (cur) {
        if (cur->free && cur->size >= need) {
            if (cur->size >= need + HDR_SZ + ALIGNMENT) {
                block *newblk = (block *)((char *)cur + HDR_SZ + need);
                newblk->size = cur->size - need - HDR_SZ;
                newblk->free = 1;
                newblk->align = 0;
                newblk->next = cur->next;
                newblk->prev = cur;
                if (cur->next) cur->next->prev = newblk;
                cur->next = newblk;
                cur->size = need;
            }
            cur->free = 0;
            cur->align = 0;
            return (void *)((char *)cur + HDR_SZ);
        }
        cur = cur->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    block *b = (block *)((char *)p - HDR_SZ);
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    size_t need = ALIGN_UP(n, ALIGNMENT);
    block *b = (block *)((char *)p - HDR_SZ);
    size_t avail = b->size + (b->next ? b->next->size + HDR_SZ : 0);
    if (b->next && b->next->free && avail >= need) {
        if (b->next->size >= need - b->size) {
            b->size = need;
            coalesce(b);
            return p;
        }
    }
    void *np = arena_alloc(need);
    if (np == NULL) return NULL;
    size_t copy = b->size < n ? b->size : n;
    memcpy(np, p, copy);
    arena_free(p);
    return np;
}