#include <stddef.h>
#include <stdalign.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN(n) (((n) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define HDR_SIZE ALIGNMENT
#define FREE_FLAG 1

typedef struct block_header {
    size_t size;
    unsigned int is_free : 1;
    unsigned int padding : 31;
} block_header_t;

static void *g_buffer_start;
static size_t g_buffer_size;

void arena_init(void *buf, size_t size) {
    g_buffer_start = buf;
    g_buffer_size = size;
    if (size < HDR_SIZE) return;
    
    block_header_t *first = (block_header_t *)buf;
    size_t usable = size - HDR_SIZE;
    first->size = usable;
    first->is_free = 1;
    first->padding = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (!g_buffer_start || g_buffer_size < HDR_SIZE) return NULL;

    size_t req_size = ALIGN(n) + HDR_SIZE;
    if (req_size > g_buffer_size) return NULL;

    block_header_t *current = (block_header_t *)g_buffer_start;
    size_t remaining = g_buffer_size;

    while (remaining > 0) {
        size_t hdr_size = HDR_SIZE;
        size_t block_size = current->size;
        size_t next_offset = hdr_size + block_size;

        if (current->is_free && block_size >= req_size) {
            if (block_size > req_size + HDR_SIZE) {
                // Split the block
                block_header_t *next_block = (block_header_t *)((char *)current + req_size);
                next_block->size = block_size - req_size;
                next_block->is_free = 1;
                next_block->padding = 0;
                current->size = req_size - hdr_size;
                current->is_free = 0;
                return (char *)current + hdr_size;
            } else {
                // Use the whole block
                current->is_free = 0;
                return (char *)current + hdr_size;
            }
        }

        remaining -= next_offset;
        current = (block_header_t *)((char *)current + next_offset);
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    if (!g_buffer_start) return;

    block_header_t *block = (block_header_t *)((char *)p - HDR_SIZE);
    size_t current_size = block->size;
    block->is_free = 1;

    // Merge with next if free
    char *next_addr = (char *)block + HDR_SIZE + current_size;
    if ((size_t)(next_addr - (char *)g_buffer_start) <= g_buffer_size) {
        block_header_t *next_block = (block_header_t *)next_addr;
        if (next_block->is_free) {
            current_size += next_block->size;
            // No need to update next_block as we are effectively removing it
        }
    }

    // Merge with previous if free
    if ((size_t)((char *)block - (char *)g_buffer_start) > 0) {
        char *prev_addr = (char *)block - HDR_SIZE;
        if ((size_t)(prev_addr - (char *)g_buffer_start) >= 0) {
            block_header_t *prev_block = (block_header_t *)prev_addr;
            if (prev_block->is_free) {
                current_size += prev_block->size;
                // prev_block is now merged into current
            }
        }
    }

    block->size = current_size;
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!g_buffer_start) return NULL;

    block_header_t *block = (block_header_t *)((char *)p - HDR_SIZE);
    size_t old_size = block->size;
    size_t req_size = ALIGN(n) + HDR_SIZE;

    if (old_size >= req_size) {
        // Block is large enough, just use it (do not shrink)
        return p;
    }

    // Try to grow into next block
    char *next_addr = (char *)block + HDR_SIZE + old_size;
    if ((size_t)(next_addr - (char *)g_buffer_start) <= g_buffer_size) {
        block_header_t *next_block = (block_header_t *)next_addr;
        if (next_block->is_free) {
            size_t combined_size = old_size + next_block->size;
            if (combined_size >= req_size) {
                // Merge and use
                block->size = combined_size;
                block->is_free = 0;
                // next_block is effectively removed
                return p;
            }
        }
    }

    // Cannot grow in place, need to allocate new
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    size_t copy_size = (n < old_size) ? n : old_size;
    if (copy_size > 0) {
        __builtin_memcpy(new_p, p, copy_size);
    }
    arena_free(p);
    return new_p;
}