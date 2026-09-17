#include <stddef.h>
#include <stdalign.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define HEADER_SIZE ((sizeof(size_t) + ALIGNMENT - 1) / ALIGNMENT * ALIGNMENT)

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
} block_header_t;

static block_header_t *head;
static size_t total_size;

void arena_init(void *buf, size_t size) {
    if (size < HEADER_SIZE + sizeof(block_header_t)) {
        head = NULL;
        total_size = 0;
        return;
    }
    // Align the buffer start for the first block header
    char *aligned_buf = (char *)buf;
    size_t offset = (HEADER_SIZE - ((size_t)aligned_buf % ALIGNMENT)) % ALIGNMENT;
    if (offset == 0) offset = 0; // Already aligned if size is multiple, but we need to ensure header alignment
    
    // Actually, the buffer passed in is the raw memory. We need to place the header at an aligned address.
    // Let's assume the buffer is large enough to hold the header aligned.
    // We will align the start of the usable area, but the header itself must be aligned.
    // Simpler approach: The buffer 'buf' is the start. We align the header pointer.
    
    char *raw = (char *)buf;
    size_t header_start = (size_t)raw;
    size_t aligned_header = (header_start + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t padding = aligned_header - header_start;
    
    if (padding + HEADER_SIZE + sizeof(block_header_t) > size) {
        head = NULL;
        total_size = 0;
        return;
    }

    block_header_t *first = (block_header_t *)(raw + padding);
    first->size = size - padding - HEADER_SIZE;
    first->is_free = 1;
    first->next = NULL;
    
    head = first;
    total_size = size;
}

static block_header_t *find_block(size_t n) {
    block_header_t *curr = head;
    while (curr) {
        if (curr->is_free && curr->size >= n) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (!head) return NULL;

    size_t aligned_n = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    if (aligned_n < HEADER_SIZE) aligned_n = HEADER_SIZE; // Ensure header fits if we split? No, header is separate.
    // Actually, the size stored in header is the payload size.
    // We need payload >= n.
    
    block_header_t *block = find_block(aligned_n);
    if (!block) return NULL;

    // Split if possible
    size_t remaining = block->size - aligned_n;
    if (remaining > HEADER_SIZE + sizeof(block_header_t)) {
        // Create new free block
        char *new_block_addr = (char *)block + HEADER_SIZE + aligned_n;
        block_header_t *new_block = (block_header_t *)new_block_addr;
        new_block->size = remaining - HEADER_SIZE;
        new_block->is_free = 1;
        new_block->next = block->next;
        block->next = new_block;
        block->size = aligned_n;
    } else {
        // Take the whole block
        block->is_free = 0;
    }

    return (void *)((char *)block + HEADER_SIZE);
}

void arena_free(void *p) {
    if (!p || !head) return;

    block_header_t *block = (block_header_t *)((char *)p - HEADER_SIZE);
    block->is_free = 1;

    // Merge with next
    if (block->next && block->next->is_free) {
        block->size += HEADER_SIZE + block->next->size;
        block->next = block->next->next;
    }

    // Merge with previous
    block_header_t *prev = head;
    while (prev && prev->next != block) {
        prev = prev->next;
    }
    if (prev && prev->is_free) {
        prev->size += HEADER_SIZE + block->size;
        prev->next = block->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);
    if (!head) return NULL;

    block_header_t *block = (block_header_t *)((char *)p - HEADER_SIZE);
    size_t old_size = block->size;
    size_t aligned_n = (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    if (aligned_n < HEADER_SIZE) aligned_n = HEADER_SIZE;

    if (aligned_n <= old_size) {
        // Can fit in place, no move needed
        return p;
    }

    // Try to grow into next block
    if (block->next && block->next->is_free) {
        size_t next_size = block->next->size;
        if (block->size + HEADER_SIZE + next_size >= aligned_n) {
            // Merge next into current
            block->size += HEADER_SIZE + next_size;
            block->next = block->next->next;
            // Copy data if needed (though we are growing, so no copy needed for existing data)
            // Just update size.
            return p;
        }
    }

    // Cannot grow in place, need to allocate new and copy
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    memcpy(new_p, p, (old_size < n) ? old_size : n);
    arena_free(p);
    return new_p;
}