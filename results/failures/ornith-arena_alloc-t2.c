#include <stddef.h>

typedef struct Block {
    size_t size;
    int free;
    struct Block *next;
} Block;

static char *g_buf;
static size_t g_size;
static Block *g_head;

static int aligned(size_t n) {
    return (n % (_Alignof(max_align_t)) == 0);
}

static void split(Block *b, size_t n) {
    if (b->size >= n + (sizeof(Block) + 1)) {
        b->size -= n + sizeof(Block);
        Block *nb = (Block *)((char *)b + sizeof(Block) + b->size);
        nb->size = n;
        nb->free = 0;
        nb->next = b->next;
        b->next = nb;
    }
}

void arena_init(void *buf, size_t size) {
    g_buf = (char *)buf;
    g_size = size;
    g_head = (Block *)g_buf;
    g_head->size = (size >= sizeof(Block)) ? (size - sizeof(Block)) : 0;
    g_head->free = 1;
    g_head->next = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return 0;
    if (!aligned(n)) n += (_Alignof(max_align_t) - (n % (_Alignof(max_align_t))));
    Block *prev = 0;
    Block *b = g_head;
    while (b) {
        if (b->free && b->size >= n) {
            if (prev) prev->next = b->next;
            else g_head = b->next;
            split(b, n);
            b->free = 0;
            return (char *)b + sizeof(Block);
        }
        prev = b;
        b = b->next;
    }
    return 0;
}

void arena_free(void *p) {
    if (!p) return;
    Block *b = (Block *)((char *)p - sizeof(Block));
    b->free = 1;
    Block *prev = 0, *cur = g_head;
    while (cur && cur < b) { prev = cur; cur = cur->next; }
    b->next = cur;
    if (prev) prev->next = b;
    else g_head = b;
    if (prev && prev->free) {
        prev->size += sizeof(Block) + prev->next->size;
        prev->next = prev->next->next;
        if (!prev->next) g_head = prev;
    }
    if (b->next && b->next->free) {
        b->size += sizeof(Block) + b->next->size;
        b->next = b->next->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return 0; }
    Block *b = (Block *)((char *)p - sizeof(Block));
    size_t old = b->size;
    if (!aligned(n)) n += (_Alignof(max_align_t) - (n % (_Alignof(max_align_t))));
    if (b->size >= n) return p;
    if (b->next && b->next->free && b->size + sizeof(Block) + b->next->size >= n) {
        char *data = (char *)b + sizeof(Block);
        b->size += sizeof(Block) + b->next->size;
        b->next = b->next->next;
        if (!b->next) g_head = b;
        size_t copy = (old < n) ? old : n;
        __builtin_memcpy(data, data, copy);
        b->size = n;
        return data;
    }
    void *np = arena_alloc(n);
    if (np) {
        size_t copy = (old < n) ? old : n;
        __builtin_memcpy(np, p, copy);
        arena_free(p);
    }
    return np;
}