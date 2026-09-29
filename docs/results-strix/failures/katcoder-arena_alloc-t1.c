#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARENA_MAGIC 0xA110CA7Eu

typedef struct alignas(max_align_t) {
    uint32_t magic;
    uint32_t size;
    uint8_t  is_free;
    uint8_t  pad[3];
} Block;

#define BLOCK_HEADER_SIZE ((size_t)(-(long)1 & (size_t)(-sizeof(Block))))
#define BLOCK_MIN_SIZE 16
#define BLOCK_DATA_OFFSET (BLOCK_HEADER_SIZE)
#define BLOCK_SIZE(b) ((b)->size)
#define BLOCK_IS_FREE(b) ((b)->is_free)
#define BLOCK_FROM_PTR(p) ((Block*)((char*)(p) - BLOCK_DATA_OFFSET))
#define PTR_FROM_BLOCK(b) ((void*)((char*)(b) + BLOCK_DATA_OFFSET))

static void *g_buf = NULL;
static size_t g_size = 0;
static Block *g_head = NULL;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    if (size < BLOCK_DATA_OFFSET + sizeof(Block)) {
        return;
    }
    g_head = (Block *)buf;
    g_head->magic = ARENA_MAGIC;
    g_head->size = (uint32_t)(size - BLOCK_DATA_OFFSET);
    g_head->is_free = 1;
}

static Block *find_free_block(size_t n) {
    Block *b = g_head;
    while (b) {
        if (BLOCK_IS_FREE(b) && BLOCK_SIZE(b) >= n) {
            return b;
        }
        b = (Block *)((char *)b + BLOCK_DATA_OFFSET + BLOCK_SIZE(b));
    }
    return NULL;
}

static void split_block(Block *b, size_t n) {
    size_t avail = BLOCK_SIZE(b);
    if (avail - n >= BLOCK_DATA_OFFSET + sizeof(Block) + BLOCK_MIN_SIZE) {
        Block *new_block = (Block *)((char *)b + BLOCK_DATA_OFFSET + n);
        new_block->magic = ARENA_MAGIC;
        new_block->size = (uint32_t)(avail - n - BLOCK_DATA_OFFSET);
        new_block->is_free = 1;
        BLOCK_SIZE(b) = (uint32_t)n;
    }
}

static void merge_blocks(Block *prev, Block *curr) {
    prev->size = (uint32_t)(BLOCK_SIZE(prev) + BLOCK_DATA_OFFSET + BLOCK_SIZE(curr));
    prev->is_free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0 || !g_buf) return NULL;
    size_t total_needed = BLOCK_DATA_OFFSET + n;
    if (total_needed < n) return NULL;
    Block *b = find_free_block(total_needed);
    if (!b) return NULL;
    split_block(b, total_needed);
    b->is_free = 0;
    b->magic = ARENA_MAGIC;
    return PTR_FROM_BLOCK(b);
}

void arena_free(void *p) {
    if (!p || !g_buf) return;
    Block *b = BLOCK_FROM_PTR(p);
    if (b->magic != ARENA_MAGIC) return;
    b->is_free = 1;
    Block *next = (Block *)((char *)b + BLOCK_DATA_OFFSET + BLOCK_SIZE(b));
    if ((char *)next < (char *)g_head + g_size && next->magic == ARENA_MAGIC) {
        if (BLOCK_IS_FREE(next)) {
            merge_blocks(b, next);
        }
    }
    if (b != g_head) {
        Block *prev = (Block *)((char *)b - BLOCK_DATA_OFFSET - ((Block *)((char *)b - BLOCK_DATA_OFFSET))->size);
        if ((char *)prev >= (char *)g_head && prev->magic == ARENA_MAGIC && BLOCK_IS_FREE(prev)) {
            merge_blocks(prev, b);
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    if (!g_buf) return NULL;
    Block *b = BLOCK_FROM_PTR(p);
    if (b->magic != ARENA_MAGIC) return NULL;
    size_t old_size = BLOCK_SIZE(b);
    size_t total_needed = BLOCK_DATA_OFFSET + n;
    if (total_needed <= old_size) {
        return p;
    }
    Block *next = (Block *)((char *)b + BLOCK_DATA_OFFSET + old_size);
    if ((char *)next < (char *)g_head + g_size && next->magic == ARENA_MAGIC && BLOCK_IS_FREE(next)) {
        size_t combined = old_size + BLOCK_DATA_OFFSET + BLOCK_SIZE(next);
        if (combined >= total_needed) {
            memcpy(PTR_FROM_BLOCK(b), p, old_size < n ? old_size : n);
            if (combined - total_needed >= BLOCK_DATA_OFFSET + sizeof(Block) + BLOCK_MIN_SIZE) {
                Block *new_block = (Block *)((char *)b + BLOCK_DATA_OFFSET + total_needed);
                new_block->magic = ARENA_MAGIC;
                new_block->size = (uint32_t)(combined - total_needed - BLOCK_DATA_OFFSET);
                new_block->is_free = 1;
            }
            BLOCK_SIZE(b) = (uint32_t)total_needed;
            b->is_free = 0;
            return p;
        }
    }
    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = old_size < n ? old_size : n;
        memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}