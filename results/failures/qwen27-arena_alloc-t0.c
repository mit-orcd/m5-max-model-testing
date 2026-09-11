#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    size_t size;
    int free;
} BlockHeader;

static void *arena_buf;
static size_t arena_size;
static size_t arena_used;

static size_t align_up(size_t n, size_t alignment) {
    return (n + alignment - 1) & ~(alignment - 1);
}

static BlockHeader *get_header(void *p) {
    return (BlockHeader *)p - 1;
}

static void *get_data(BlockHeader *h) {
    return (void *)(h + 1);
}

static BlockHeader *next_block(BlockHeader *h) {
    return (BlockHeader *)((char *)h + h->size);
}

static void merge_with_next(BlockHeader *h) {
    BlockHeader *next = next_block(h);
    if ((char *)next < (char *)arena_buf + arena_size && next->free) {
        h->size += next->size;
    }
}

static void merge_with_prev(BlockHeader *h) {
    if ((char *)h > (char *)arena_buf) {
        BlockHeader *prev = (BlockHeader *)((char *)h - sizeof(BlockHeader));
        if (prev->free) {
            prev->size += h->size;
        }
    }
}

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_used = 0;
    
    if (buf == NULL || size < sizeof(BlockHeader) + _Alignof(max_align_t)) {
        return;
    }
    
    BlockHeader *h = (BlockHeader *)buf;
    h->size = size;
    h->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }
    
    if (arena_buf == NULL) {
        return NULL;
    }
    
    size_t aligned_n = align_up(n, _Alignof(max_align_t));
    size_t total_size = sizeof(BlockHeader) + aligned_n;
    
    if (total_size > arena_size) {
        return NULL;
    }
    
    BlockHeader *h = (BlockHeader *)arena_buf;
    while ((char *)h < (char *)arena_buf + arena_size) {
        if (h->free && h->size >= total_size) {
            if (h->size > total_size + _Alignof(max_align_t)) {
                BlockHeader *split = (BlockHeader *)((char *)h + total_size);
                split->size = h->size - total_size;
                split->free = 1;
                h->size = total_size;
            }
            h->free = 0;
            return get_data(h);
        }
        h = next_block(h);
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }
    
    if (arena_buf == NULL) {
        return;
    }
    
    if ((char *)p < (char *)arena_buf || (char *)p >= (char *)arena_buf + arena_size) {
        return;
    }
    
    BlockHeader *h = get_header(p);
    h->free = 1;
    
    merge_with_next(h);
    merge_with_prev(h);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    if (arena_buf == NULL) {
        return NULL;
    }
    
    if ((char *)p < (char *)arena_buf || (char *)p >= (char *)arena_buf + arena_size) {
        return NULL;
    }
    
    BlockHeader *h = get_header(p);
    size_t old_size = h->size - sizeof(BlockHeader);
    size_t aligned_n = align_up(n, _Alignof(max_align_t));
    size_t new_total = sizeof(BlockHeader) + aligned_n;
    
    if (h->size >= new_total) {
        if (h->size > new_total + _Alignof(max_align_t)) {
            BlockHeader *split = (BlockHeader *)((char *)h + new_total);
            split->size = h->size - new_total;
            split->free = 1;
            h->size = new_total;
        }
        return p;
    }
    
    BlockHeader *next = next_block(h);
    if ((char *)next < (char *)arena_buf + arena_size && next->free && 
        h->size + next->size >= new_total) {
        h->size += next->size;
        if (h->size > new_total + _Alignof(max_align_t)) {
            BlockHeader *split = (BlockHeader *)((char *)h + new_total);
            split->size = h->size - new_total;
            split->free = 1;
            h->size = new_total;
        }
        return p;
    }
    
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }
    
    size_t copy_size = old_size < n ? old_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    
    return new_p;
}