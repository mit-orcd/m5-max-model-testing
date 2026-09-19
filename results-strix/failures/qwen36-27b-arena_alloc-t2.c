```c
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 
 * Arena Allocator Implementation
 * 
 * Structure of the buffer:
 * The buffer is divided into blocks. Each block starts with a header.
 * The header is aligned to max_align_t.
 * 
 * Header structure:
 * - size: size of the payload (excluding the header). If the block is free, this includes the size of the next free block's header if they are merged? 
 *   Actually, standard approach: size is the payload size. The block's total size is sizeof(header) + size.
 * - next: pointer to the next block in the free list (if free) or just used for debugging? 
 *   Let's use a free list.
 * - prev: pointer to the previous block (for coalescing).
 * 
 * To simplify, let's use a implicit free list with explicit free list for O(1) allocation?
 * Or just a simple implicit free list with a header/footer for coalescing.
 * 
 * Let's use an explicit free list with a header that contains:
 * - size: the size of the usable memory in this block.
 * - next: pointer to the next free block.
 * - prev: pointer to the previous block (in memory order) to allow coalescing.
 * 
 * However, the problem states "bookkeeping must live inside that buffer".
 * 
 * Let's define a header struct. It must be aligned to max_align_t.
 */

typedef struct arena_header {
    size_t size;          // Size of the payload area following the header.
    struct arena_header *next; // Next free block in the free list.
    struct arena_header *prev; // Previous block in memory order (for coalescing).
} arena_header_t;

// Global state for the arena
static void *arena_buf_start = NULL;
static size_t arena_buf_size = 0;
static arena_header_t *free_list = NULL;

// Helper to get the header from a user pointer
static inline arena_header_t *get_header(void *p) {
    return (arena_header_t *)((uintptr_t)p - sizeof(arena_header_t));
}

// Helper to get the user pointer from a header
static inline void *get_payload(arena_header_t *h) {
    return (void *)((uintptr_t)h + sizeof(arena_header_t));
}

// Helper to get the next block in memory order
static inline arena_header_t *get_next_block_in_memory(arena_header_t *h) {
    uintptr_t next_addr = (uintptr_t)h + sizeof(arena_header_t) + h->size;
    if (next_addr + sizeof(arena_header_t) > (uintptr_t)arena_buf_start + arena_buf_size) {
        return NULL;
    }
    return (arena_header_t *)next_addr;
}

// Helper to check if a pointer is within the arena
static inline int is_in_arena(void *p) {
    return p >= arena_buf_start && 
           (uintptr_t)p < (uintptr_t)arena_buf_start + arena_buf_size;
}

void arena_init(void *buf, size_t size) {
    arena_buf_start = buf;
    arena_buf_size = size;
    free_list = NULL;

    if (buf == NULL || size == 0) {
        return;
    }

    // Align the start of the first block to max_align_t
    uintptr_t start_addr = (uintptr_t)buf;
    size_t align = _Alignof(max_align_t);
    if (start_addr % align != 0) {
        start_addr = (start_addr + align - 1) & ~(align - 1);
    }

    // If the buffer is too small to hold a header and at least some payload, we can't do much.
    // We need at least sizeof(arena_header_t) + 1 byte to be useful, but let's just check if we can fit a header.
    if (start_addr + sizeof(arena_header_t) >= (uintptr_t)buf + size) {
        return;
    }

    // Create the initial free block
    arena_header_t *h = (arena_header_t *)start_addr;
    h->size = (size_t)((uintptr_t)buf + size - (uintptr_t)h - sizeof(arena_header_t));
    h->next = NULL;
    h->prev = NULL;

    free_list = h;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    if (arena_buf_start == NULL || arena_buf_size == 0) {
        return NULL;
    }

    // Align the request size to max_align_t to ensure the next block is aligned
    size_t align = _Alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);

    // Search the free list for a suitable block
    arena_header_t *current = free_list;
    arena_header_t *prev_free = NULL;

    while (current != NULL) {
        if (current->size >= aligned_n) {
            // Found a suitable block
            void *payload = get_payload(current);
            
            // Check if we can split the block
            size_t remaining = current->size - aligned_n;
            // We need enough space for a header in the remaining part to split
            if (remaining > sizeof(arena_header_t) + align) {
                // Split the block
                arena_header_t *new_block = (arena_header_t *)((uintptr_t)payload + aligned_n);
                new_block->size = remaining - sizeof(arena_header_t);
                new_block->next = current->next;
                new_block->prev = current->prev; // The new block takes the place in memory order, but for free list, it replaces current
                
                // Update the free list
                if (prev_free == NULL) {
                    free_list = new_block;
                } else {
                    prev_free->next = new_block;
                }
                
                // Update the current block's size
                current->size = aligned_n;
                current->next = NULL; // It's no longer in the free list
                current->prev = NULL; // It's no longer in the free list, but we need to update neighbors?
                // Actually, for coalescing, we need to update the prev/next pointers of adjacent blocks.
                // The new_block is now free, so its prev should point to current? No, current is allocated.
                // The new_block's prev should point to the block before current in memory order?
                // Let's reconsider the prev pointer. It should point to the previous block in memory order, regardless of free/allocated status.
                // This is crucial for coalescing.
                
                // Let's fix the prev/next pointers for coalescing.
                // current->prev should remain as is (it points to the previous block in memory).
                // new_block->prev should point to current.
                // new_block->next should point to the block after new_block in memory.
                // But we don't have a direct way to get the next block in memory from current->next in the free list.
                // We need to traverse memory order to find the next block.
                
                // This is getting complicated. Let's simplify.
                // Instead of a free list, let's use an implicit free list with a header and footer.
                // But the problem requires merging free blocks.
                // Let's stick to the free list but ensure prev/next pointers are correct for coalescing.
                
                // For now, let's just remove the current block from the free list and return the payload.
                // We'll handle coalescing in arena_free.
                
                // Remove current from free list
                if (prev_free == NULL) {
                    free_list = current->next;
                } else {
                    prev_free->next = current->next;
                }
                
                // If we split, we need to add the new_block to the free list
                if (remaining > sizeof(