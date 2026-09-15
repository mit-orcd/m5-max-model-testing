#include <stddef.h>
#include <stdint.h>

#define ARENA_MAGIC 0xA110CA7E

typedef struct alignas(max_align_t) Block {
    uint32_t magic;
    size_t size;
    int free;
    struct Block *next;
} Block;

static void *g_buf;
static size_t g_size;
static Block *g_head;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    g_head = (Block *)buf;
    g_head->magic = ARENA_MAGIC;
    g_head->size = size - sizeof(Block);
    g_head->free = 0;
    g_head->next = NULL;
}

static void *coalesce(Block *b) {
    size_t total = b->size;
    while (b->next && b->next->free) {
        total += b->next->size + sizeof(Block);
        b->next = b->next->next;
    }
    b->size = total;
    return b;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = _Alignof(max_align_t);
    size_t aligned = (n + align - 1) & ~(align - 1);
    Block *b = g_head;
    while (b) {
        if (b->free && b->size >= aligned) {
            if (b->size >= aligned + sizeof(Block) + 1) {
                size_t rem = b->size - aligned - sizeof(Block);
                Block *split = (Block *)((char *)b + sizeof(Block) + aligned);
                split->magic = ARENA_MAGIC;
                split->size = rem;
                split->free = 1;
                split->next = b->next;
                b->size = aligned;
                b->next = split;
            }
            b->free = 0;
            return (void *)((char *)b + sizeof(Block));
        }
        b = b->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    Block *b = (Block *)((char *)p - sizeof(Block));
    if (b->magic != ARENA_MAGIC) return;
    b->free = 1;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    Block *b = (Block *)((char *)p - sizeof(Block));
    if (b->magic != ARENA_MAGIC) return NULL;
    size_t align = _Alignof(max_align_t);
    size_t aligned = (n + align - 1) & ~(align - 1);
    size_t old_size = b->size;
    if (b->size >= aligned) {
        if (b->next && b->next->free && (b->size + b->next->size + sizeof(Block)) >= aligned) {
            size_t new_size = b->size + b->next->size + sizeof(Block);
            if (new_size >= aligned + sizeof(Block) + 1) {
                size_t rem = new_size - aligned - sizeof(Block);
                Block *split = (Block *)((char *)b + sizeof(Block) + aligned);
                split->magic = ARENA_MAGIC;
                split->size = rem;
                split->free = 1;
                split->next = b->next->next;
                b->size = aligned;
                b->next = split;
            } else {
                b->size = new_size;
                b->next = b->next->next;
            }
        } else {
            b->size = aligned;
        }
        if (old_size < n) {
            char *dst = (char *)p;
            char *src = (char *)p;
            size_t copy = old_size < n ? old_size : n;
            if (copy > 0) __builtin_memcpy(dst, src, copy);
        }
        return p;
    }
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy = old_size < n ? old_size : n;
        __builtin_memcpy(new_p, p, copy);
        arena_free(p);
    }
    return new_p;
}