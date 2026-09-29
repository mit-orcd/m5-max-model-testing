#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

#define BLOCK_HEADER_SIZE ALIGN_UP(sizeof(struct block_header))
#define MIN_BLOCK_SIZE (BLOCK_HEADER_SIZE + ALIGNMENT)

struct block_header {
    size_t size;              // size of the entire block (including header)
    bool is_free;
    struct block_header *prev_free;  // previous free block in the free list
    struct block_header *next_free;  // next free block in the free list
};

static struct block_header *arena_start = NULL;
static struct block_header *free_list_head = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < MIN_BLOCK_SIZE) {
        arena_start = NULL;
        free_list_head = NULL;
        arena_size = 0;
        return;
    }

    // Align the buffer start
    uintptr_t start_addr = (uintptr_t)buf;
    uintptr_t aligned_start = (start_addr + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
    size_t offset = aligned_start - start_addr;

    if (offset + MIN_BLOCK_SIZE > size) {
        arena_start = NULL;
        free_list_head = NULL;
        arena_size = 0;
        return;
    }

    char *arena_base = (char *)buf;
    arena_start = (struct block_header *)(arena_base + offset);
    arena_size = size - offset;

    // Initialize single free block
    arena_start->size = arena_size;
    arena_start->is_free = true;
    arena_start->prev_free = NULL;
    arena_start->next_free = NULL;
    free_list_head = arena_start;
}

static struct block_header *find_free_block(size_t size) {
    struct block_header *block = free_list_head;
    while (block) {
        if (block->size >= size) {
            return block;
        }
        block = block->next_free;
    }
    return NULL;
}

static void remove_from_free_list(struct block_header *block) {
    if (!block || !block->is_free) return;

    if (block == free_list_head) {
        free_list_head = block->next_free;
    }

    if (block->prev_free) {
        block->prev_free->next_free = block->next_free;
    }

    if (block->next_free) {
        block->next_free->prev_free = block->prev_free;
    }

    block->prev_free = NULL;
    block->next_free = NULL;
}

static void insert_into_free_list(struct block_header *block) {
    if (!block) return;

    // Insert in address order to facilitate coalescing
    if (!free_list_head) {
        free_list_head = block;
        block->prev_free = NULL;
        block->next_free = NULL;
        return;
    }

    // Find insertion point
    struct block_header *current = free_list_head;
    struct block_header *prev = NULL;
    while (current && (uintptr_t)current < (uintptr_t)block) {
        prev = current;
        current = current->next_free;
    }

    block->prev_free = prev;
    block->next_free = current;

    if (prev) {
        prev->next_free = block;
    } else {
        free_list_head = block;
    }

    if (current) {
        current->prev_free = block;
    }
}

static void coalesce_block(struct block_header *block) {
    if (!block || block->is_free) {
        // Already free, coalesce with neighbors
        struct block_header *next = (struct block_header *)((char *)block + block->size);

        // Try to merge with next block
        if ((char *)next < (char *)arena_start + arena_size &&
            next->is_free &&
            (char *)next == (char *)block + block->size) {
            block->size += next->size;
            remove_from_free_list(next);
        }

        // Try to merge with previous block
        if (block != arena_start) {
            struct block_header *prev = (struct block_header *)((char *)block - sizeof(struct block_header));
            // This is tricky because we don't store back-pointer in header
            // We need to scan the arena to find prev
            // But for simplicity, we'll check adjacent memory location
            // Actually, this approach is error-prone. We'll use a different method.

            // Alternative: scan free list for adjacent block
            // But better: store a way to find previous block
            // Let's instead coalesce when freeing, and only check next block here
            // For simplicity in this implementation, we'll only merge with next block
        }
        return;
    }

    // If block is allocated, try to merge with free neighbors
    struct block_header *next = (struct block_header *)((char *)block + block->size);
    struct block_header *prev = arena_start;

    // Find prev block that ends where block starts
    while ((char *)prev + prev->size <= (char *)block) {
        if ((char *)prev + prev->size == (char *)block) {
            break;
        }
        if ((char *)prev + prev->size > (char *)block) {
            prev = NULL;
            break;
        }
        prev = (struct block_header *)((char *)prev + prev->size);
        if ((char *)prev >= (char *)arena_start + arena_size) {
            prev = NULL;
            break;
        }
    }

    if (prev && prev->is_free && (char *)prev + prev->size == (char *)block) {
        remove_from_free_list(prev);
        prev->size += block->size;
        block = prev;
    }

    if (next < (struct block_header *)((char *)arena_start + arena_size) &&
        next->is_free &&
        (char *)next == (char *)block + block->size) {
        remove_from_free_list(next);
        block->size += next->size;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0 || !arena_start) {
        return NULL;
    }

    // Adjust size for header and alignment
    size_t total_size = ALIGN_UP(n) + BLOCK_HEADER_SIZE;
    if (total_size < MIN_BLOCK_SIZE) {
        total_size = MIN_BLOCK_SIZE;
    }

    // Find suitable free block
    struct block_header *block = find_free_block(total_size);
    if (!block) {
        return NULL;
    }

    // Remove from free list
    remove_from_free_list(block);

    // Split block if large enough
    size_t remaining = block->size - total_size;
    if (remaining >= MIN_BLOCK_SIZE) {
        struct block_header *new_block = (struct block_header *)((char *)block + total_size);
        new_block->size = remaining;
        new_block->is_free = true;
        new_block->prev_free = NULL;
        new_block->next_free = NULL;

        block->size = total_size;

        // Insert new block into free list
        insert_into_free_list(new_block);
    }

    // Mark as allocated
    block->is_free = false;

    return (void *)((char *)block + BLOCK_HEADER_SIZE);
}

void arena_free(void *p) {
    if (!p) return;

    // Get block header
    struct block_header *block = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);

    // Validate pointer is within arena
    if ((char *)block < (char *)arena_start || 
        (char *)block + block->size > (char *)arena_start + arena_size) {
        return;
    }

    // Validate block is allocated
    if (block->is_free) return;

    // Mark as free
    block->is_free = true;

    // Coalesce with neighbors
    coalesce_block(block);

    // Insert into free list if not already there
    if (!block->prev_free && !block->next_free) {
        insert_into_free_list(block);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    struct block_header *block = (struct block_header *)((char *)p - BLOCK_HEADER_SIZE);

    // Validate pointer
    if ((char *)block < (char *)arena_start || 
        (char *)block + block->size > (char *)arena_start + arena_size) {
        return NULL;
    }

    // Check if block is allocated
    if (block->is_free) return NULL;

    size_t aligned_n = ALIGN_UP(n);
    size_t needed = aligned_n + BLOCK_HEADER_SIZE;
    if (needed < MIN_BLOCK_SIZE) needed = MIN_BLOCK_SIZE;

    // Check if we can grow in place
    struct block_header *next = (struct block_header *)((char *)block + block->size);

    // Check if next block is free and large enough
    if ((char *)next < (char *)arena_start + arena_size &&
        next->is_free &&
        block->size + next->size >= needed) {
        // Merge with next block
        remove_from_free_list(next);
        block->size += next->size;

        // Split if necessary
        size_t remaining = block->size - needed;
        if (remaining >= MIN_BLOCK_SIZE) {
            struct block_header *new_block = (struct block_header *)((char *)block + needed);
            new_block->size = remaining;
            new_block->is_free = true;
            new_block->prev_free = NULL;
            new_block->next_free = NULL;
            block->size = needed;
            insert_into_free_list(new_block);
        }

        return p;
    }

    // Check if we can shrink
    if (block->size >= needed + MIN_BLOCK_SIZE) {
        // Shrink block
        size_t remaining = block->size - needed;
        struct block_header *new_block = (struct block_header *)((char *)block + needed);
        new_block->size = remaining;
        new_block->is_free = true;
        new_block->prev_free = NULL;
        new_block->next_free = NULL;
        block->size = needed;
        insert_into_free_list(new_block);
        return p;
    }

    // Must allocate new block and copy
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) {
        return NULL;
    }

    // Copy data
    size_t old_size = block->size - BLOCK_HEADER_SIZE;
    size_t copy_size = old_size < aligned_n ? old_size : aligned_n;
    memcpy(new_ptr, p, copy_size);

    arena_free(p);
    return new_ptr;
}