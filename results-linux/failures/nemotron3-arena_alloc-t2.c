#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define MIN_BLOCK_SIZE (2 * sizeof(size_t))

typedef struct {
    size_t size;      /* total size of block (header + payload), includes alignment */
    int free;         /* 1 if free, 0 if allocated */
} BlockHeader;

static void *heap_start = NULL;
static size_t heap_size = 0;

static BlockHeader *get_header(void *p) {
    return (BlockHeader *)((uint8_t *)p - sizeof(BlockHeader));
}

static void *get_payload(BlockHeader *h) {
    return (void *)((uint8_t *)h + sizeof(BlockHeader));
}

static BlockHeader *next_block(BlockHeader *h) {
    return (BlockHeader *)((uint8_t *)h + h->size);
}

static BlockHeader *prev_block(BlockHeader *h) {
    if ((uint8_t *)h == (uint8_t *)heap_start) return NULL;
    BlockHeader *prev = (BlockHeader *)((uint8_t *)h - sizeof(size_t));
    size_t prev_size = *prev;
    return (BlockHeader *)((uint8_t *)h - prev_size);
}

static void set_size(BlockHeader *h, size_t s) {
    h->size = s;
    if ((uint8_t *)h + s < (uint8_t *)heap_start + heap_size) {
        BlockHeader *next = next_block(h);
        *(size_t *)next = s; /* store size in prev block's footer */
    }
}

static void set_free(BlockHeader *h, int f) {
    h->free = f;
}

static int get_free(BlockHeader *h) {
    return h->free;
}

static size_t get_block_size(BlockHeader *h) {
    return h->size;
}

static void coalesce_with_next(BlockHeader *h) {
    BlockHeader *next = next_block(h);
    if ((uint8_t *)next >= (uint8_t *)heap_start + heap_size) return;
    if (!get_free(next)) return;
    size_t new_size = get_block_size(h) + get_block_size(next);
    set_size(h, new_size);
    set_free(h, 1);
}

static void coalesce_with_prev(BlockHeader *h) {
    BlockHeader *prev = prev_block(h);
    if (!prev) return;
    if (!get_free(prev)) return;
    size_t new_size = get_block_size(prev) + get_block_size(h);
    set_size(prev, new_size);
    set_free(prev, 1);
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(BlockHeader) + MIN_BLOCK_SIZE) {
        heap_start = NULL;
        heap_size = 0;
        return;
    }
    heap_start = buf;
    heap_size = size;
    BlockHeader *h = (BlockHeader *)heap_start;
    set_size(h, size);
    set_free(h, 1);
    /* Initialize footer */
    if (size >= sizeof(BlockHeader)) {
        *(size_t *)((uint8_t *)h + size - sizeof(size_t)) = size;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t needed = ALIGN_UP(n) + sizeof(BlockHeader);
    if (needed < MIN_BLOCK_SIZE) needed = MIN_BLOCK_SIZE;

    BlockHeader *curr = (BlockHeader *)heap_start;
    while ((uint8_t *)curr < (uint8_t *)heap_start + heap_size) {
        if (get_free(curr) && get_block_size(curr) >= needed) {
            size_t block_size = get_block_size(h);
            size_t remainder = block_size - needed;
            set_size(curr, needed);
            set_free(curr, 0);
            /* Initialize footer for allocated block */
            *(size_t *)((uint8_t *)curr + needed - sizeof(size_t)) = needed;

            if (remainder >= sizeof(BlockHeader) + MIN_BLOCK_SIZE) {
                BlockHeader *next = next_block(curr);
                set_size(next, remainder);
                set_free(next, 1);
                *(size_t *)((uint8_t *)next + remainder - sizeof(size_t)) = remainder;
            }
            return get_payload(curr);
        }
        curr = next_block(curr);
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    BlockHeader *h = get_header(p);
    set_free(h, 1);
    /* Initialize footer for free block */
    *(size_t *)((uint8_t *)h + get_block_size(h) - sizeof(size_t)) = get_block_size(h);
    coalesce_with_prev(h);
    coalesce_with_next(h);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    BlockHeader *h = get_header(p);
    size_t old_payload = get_block_size(h) - sizeof(BlockHeader);
    size_t needed = ALIGN_UP(n) + sizeof(BlockHeader);
    if (needed < MIN_BLOCK_SIZE) needed = MIN_BLOCK_SIZE;

    if (get_block_size(h) >= needed) {
        size_t remainder = get_block_size(h) - needed;
        set_size(h, needed);
        set_free(h, 0);
        *(size_t *)((uint8_t *)h + needed - sizeof(size_t)) = needed;
        if (remainder >= sizeof(BlockHeader) + MIN_BLOCK_SIZE) {
            BlockHeader *next = next_block(h);
            set_size(next, remainder);
            set_free(next, 1);
            *(size_t *)((uint8_t *)next + remainder - sizeof(size_t)) = remainder;
        }
        return p;
    }

    BlockHeader *next = next_block(h);
    if ((uint8_t *)next < (uint8_t *)heap_start + heap_size && get_free(next)) {
        size_t combined = get_block_size(h) + get_block_size(next);
        if (combined >= needed) {
            set_size(h, combined);
            set_free(h, 0);
            *(size_t *)((uint8_t *)h + combined - sizeof(size_t)) = combined;
            size_t remainder = combined - needed;
            if (remainder >= sizeof(BlockHeader) + MIN_BLOCK_SIZE) {
                BlockHeader *new_next = next_block(h);
                set_size(new_next, remainder);
                set_free(new_next, 1);
                *(size_t *)((uint8_t *)new_next + remainder - sizeof(size_t)) = remainder;
            }
            return p;
        }
    }

    void *newp = arena_alloc(n);
    if (!newp) return NULL;
    size_t copy_size = old_payload < n ? old_payload : n;
    memcpy(newp, p, copy_size);
    arena_free(p);
    return newp;
}