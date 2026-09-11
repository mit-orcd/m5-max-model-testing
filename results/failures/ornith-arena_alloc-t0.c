#include <stddef.h>

typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block;

static char *g_buf;
static size_t g_size;
static block *g_head;

void arena_init(void *buf, size_t size) {
    g_buf = (char *)buf;
    g_size = size;
    g_head = (block *)g_buf;
    g_head->size = size;
    g_head->free = 1;
    g_head->next = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return 0;
    size_t need = n + sizeof(block);
    block *cur = g_head;
    while (cur) {
        if (cur->free && cur->size >= need) {
            if (cur->size >= need + sizeof(block) + 1) {
                block *split = (block *)(cur + 1);
                split->size = cur->size - need;
                split->free = 1;
                split->next = cur->next;
                cur->size = need;
                cur->next = split;
            } else {
                cur->free = 0;
            }
            return (void *)((char *)cur + sizeof(block));
        }
        cur = cur->next;
    }
    return 0;
}

void arena_free(void *p) {
    if (!p) return;
    block *b = (block *)((char *)p - sizeof(block));
    b->free = 1;
    if (b->next && b->next->free) {
        b->size += b->next->size;
        b->next = b->next->next;
    }
    if (b != g_head && b->next && b->free) {
        block *cur = g_head;
        while (cur && cur->next != b) cur = cur->next;
        if (cur && cur->free) {
            cur->size += b->size;
            cur->next = b->next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return 0; }
    block *b = (block *)((char *)p - sizeof(block));
    size_t old = b->size - sizeof(block);
    if (b->size >= sizeof(block) + n) return p;
    size_t need = n + sizeof(block);
    if (b->next && b->next->free && (b->size + b->next->size) >= need) {
        size_t extra = b->next->size;
        if (extra >= need + sizeof(block) + 1) {
            block *split = (block *)((char *)b + need);
            split->size = extra - need;
            split->free = 1;
            split->next = b->next->next;
            b->size = need;
            b->next = split;
        } else {
            b->size += extra;
            b->next = b->next->next;
        }
        return p;
    }
    void *np = arena_alloc(n);
    if (np) {
        size_t copy = old < n ? old : n;
        __builtin_memcpy(np, p, copy);
        arena_free(p);
    }
    return np;
}