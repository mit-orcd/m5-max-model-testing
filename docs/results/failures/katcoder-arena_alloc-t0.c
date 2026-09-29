#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN(n) (((n) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct Block {
    size_t size;
    int free;
} Block;

static void *g_buf;
static size_t g_size;
static size_t g_total;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    g_total = 0;
    if (size >= sizeof(Block)) {
        Block *b = (Block *)buf;
        b->size = size - sizeof(Block);
        b->free = 1;
    }
}

static Block *first_block(void) {
    return (Block *)g_buf;
}

static void *block_data(Block *b) {
    return (void *)((char *)b + sizeof(Block));
}

static Block *next_block(Block *b) {
    return (Block *)((char *)b + sizeof(Block) + b->size);
}

static int block_is_end(Block *b) {
    return (char *)next_block(b) >= (char *)g_buf + g_size;
}

static void coalesce(Block *b) {
    Block *n = next_block(b);
    if (!block_is_end(n) && n->free) {
        b->size += sizeof(Block) + n->size;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t aligned = ALIGN(n);
    Block *b = first_block();
    while (!block_is_end(b)) {
        if (b->free && b->size >= aligned) {
            if (b->size >= aligned + sizeof(Block) + ALIGNMENT) {
                Block *rest = (Block *)((char *)b + sizeof(Block) + aligned);
                rest->size = b->size - aligned - sizeof(Block);
                rest->free = 1;
                b->size = aligned;
                coalesce(b);
            }
            b->free = 0;
            g_total += b->size;
            return block_data(b);
        }
        b = next_block(b);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    Block *b = (Block *)((char *)p - sizeof(Block));
    b->free = 1;
    g_total -= b->size;
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    Block *b = (Block *)((char *)p - sizeof(Block));
    size_t old_size = b->size;
    size_t aligned = ALIGN(n);
    if (b->size >= aligned) {
        if (b->size >= aligned + sizeof(Block) + ALIGNMENT) {
            Block *rest = (Block *)((char *)b + sizeof(Block) + aligned);
            rest->size = b->size - aligned - sizeof(Block);
            rest->free = 1;
            b->size = aligned;
            coalesce(b);
        } else {
            b->size = aligned;
            coalesce(b);
        }
        if (n < old_size) {
            /* preserve contents up to smaller size */
        }
        return p;
    }
    Block *next = next_block(b);
    if (!block_is_end(next) && next->free) {
        size_t combined = b->size + sizeof(Block) + next->size;
        if (combined >= aligned) {
            if (combined >= aligned + sizeof(Block) + ALIGNMENT) {
                Block *rest = (Block *)((char *)b + sizeof(Block) + aligned);
                rest->size = combined - aligned - sizeof(Block);
                rest->free = 1;
            }
            b->size = aligned;
            coalesce(b);
            size_t copy = n < old_size ? n : old_size;
            return p;
        }
    }
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    size_t copy = n < old_size ? n : old_size;
    __builtin_memcpy(new_p, p, copy);
    arena_free(p);
    return new_p;
}