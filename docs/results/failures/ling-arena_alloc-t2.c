#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

static char *arena_base;
static size_t arena_size;

typedef struct BlockHeader {
    size_t size;
    uint8_t free;
} BlockHeader;

static BlockHeader *next_header(const BlockHeader *block) {
    return (BlockHeader *)((uintptr_t)block + sizeof(BlockHeader) + block->size);
}

static BlockHeader *block_from_ptr(const void *p) {
    return (BlockHeader *)((uintptr_t)p - sizeof(BlockHeader));
}

static BlockHeader *find_free_block(size_t n) {
    char *cur = arena_base;
    char *end = arena_base + arena_size;
    while (cur + sizeof(BlockHeader) < end) {
        BlockHeader *h = (BlockHeader *)cur;
        if (h->free && h->size >= n) return h;
        cur += sizeof(BlockHeader) + h->size;
    }
    return NULL;
}

void arena_init(void *buf, size_t size) {
    arena_base = (char *)buf;
    arena_size = size;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t asize = ALIGN_UP(n);
    BlockHeader *b = find_free_block(asize);
    if (!b) return NULL;
    b->free = 0;
    return (void *)((uintptr_t)b + sizeof(BlockHeader));
}

void arena_free(void *p) {
    if (!p) return;
    BlockHeader *b = block_from_ptr(p);
    b->free = 1;
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    BlockHeader *b = block_from_ptr(p);
    size_t asize = ALIGN_UP(n);

    char *next_start = (char *)next_header(b);
    size_t combined = sizeof(BlockHeader) + b->size;
    if (!b->free && (size_t)(next_start - arena_base) < arena_size) {
        BlockHeader *nh = next_header(b);
        if (nh->free) combined += sizeof(BlockHeader) + nh->size;
    }

    if (combined >= sizeof(BlockHeader) + asize) {
        if (combined > sizeof(BlockHeader) + asize) {
            size_t remaining = combined - (sizeof(BlockHeader) + asize);
            BlockHeader *new_next = (BlockHeader *)((uintptr_t)b + sizeof(BlockHeader) + asize);
            new_next->size = remaining - sizeof(BlockHeader);
            new_next->free = 1;
            b->size = asize;
        }
        b->free = 0;
        return p;
    }

    void *np = arena_alloc(asize);
    if (!np) return NULL;
    memcpy(np, p, (b->size < asize) ? b->size : asize);
    arena_free(p);
    return np;
}