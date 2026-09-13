#include <stddef.h>
#include <stdint.h>

#define ARENA_MAGIC 0xA110CA7E

typedef struct {
    uint32_t magic;
    size_t size;
    uint8_t free;
} block_header_t;

static void *g_buf = NULL;
static size_t g_size = 0;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    
    if (size == 0) return;

    block_header_t *first = (block_header_t *)buf;
    first->magic = ARENA_MAGIC;
    first->size = size - sizeof(block_header_t);
    first->free = 1;
}

static inline void *next_block(void *p) {
    block_header_t *hdr = (block_header_t *)p;
    return (void *)((uint8_t *)p + sizeof(block_header_t) + hdr->size);
}

static inline int is_valid(void *p, size_t total_size) {
    return (uint8_t *)p >= (uint8_t *)g_buf && 
           (uint8_t *)p + sizeof(block_header_t) <= (uint8_t *)g_buf + total_size;
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_buf == NULL || g_size == 0) return NULL;

    // Align size to 16 bytes (max_align_t is usually 16, but let's be safe with 8 or 16)
    // Standard max_align_t is often 16. Let's use 16.
    size_t align = 16;
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    size_t header_size = sizeof(block_header_t);
    
    // Minimum block size: header + aligned data
    size_t min_block_size = header_size + aligned_n;
    
    if (min_block_size > g_size) return NULL;

    block_header_t *current = (block_header_t *)g_buf;
    block_header_t *best = NULL;
    size_t best_size = 0;

    while (current < (block_header_t *)((uint8_t *)g_buf + g_size)) {
        if (current->magic != ARENA_MAGIC) return NULL; // Corruption
        
        if (current->free) {
            if (current->size >= min_block_size) {
                if (best == NULL || current->size < best_size) {
                    best = current;
                    best_size = current->size;
                }
                // First fit optimization: just take the first one that fits
                // The prompt doesn't specify best-fit vs first-fit, but "reusable" implies fragmentation handling.
                // First fit is standard for simple arenas.
                break; 
            }
        }
        
        if (current->size + header_size > current->size) { // Check for overflow
             current = (block_header_t *)((uint8_t *)current + header_size + current->size);
        } else {
             // Overflow or invalid size, stop
             break;
        }
    }

    if (best == NULL) return NULL;

    // Split block if remaining space is large enough for another block header + min data
    size_t remaining = best->size - min_block_size;
    // Let's say min allocatable data is 1 byte, so min block is sizeof(header) + 1.
    // Actually, let's just split if remaining >= sizeof(block_header_t) + 1.
    // Or simpler: just split if remaining >= sizeof(block_header_t) + sizeof(max_align_t).
    // To keep it simple and robust, split if remaining >= sizeof(block_header_t) + 1.
    
    if (remaining >= sizeof(block_header_t)) {
        // Create a new free block after the allocated part
        uint8_t *new_block_addr = (uint8_t *)best + sizeof(block_header_t) + min_block_size;
        block_header_t *new_free = (block_header_t *)new_block_addr;
        new_free->magic = ARENA_MAGIC;
        new_free->size = remaining - sizeof(block_header_t);
        new_free->free = 1;
        
        // Update current block
        best->free = 0;
        best->size = min_block_size;
        
        return (void *)(best + 1);
    } else {
        // Use the whole block, mark as allocated
        best->free = 0;
        return (void *)(best + 1);
    }
}

void arena_free(void *p) {
    if (p == NULL) return;

    block_header_t *block = (block_header_t *)p - 1;
    
    if (!is_valid(p, g_size)) return;
    if (block->magic != ARENA_MAGIC) return;
    if (block->free) return; // Already free

    block->free = 1;

    // Merge with next block if it exists and is free
    uint8_t *end = (uint8_t *)block + sizeof(block_header_t) + block->size;
    if (end < (uint8_t *)g_buf + g_size) {
        block_header_t *next = (block_header_t *)end;
        if (next->magic == ARENA_MAGIC && next->free) {
            block->size += sizeof(block_header_t) + next->size;
        }
    }

    // Merge with previous block if it exists and is free
    if (block > (block_header_t *)g_buf) {
        block_header_t *prev = (block_header_t *)((uint8_t *)block - sizeof(block_header_t));
        // Check if prev is within bounds and valid
        if (prev->magic == ARENA_MAGIC && prev->free) {
            prev->size += sizeof(block_header_t) + block->size;
            // No need to mark block as invalid, it's logically merged into prev.
            // However, subsequent allocs landing in this space will use prev's header.
            // We don't need to do anything else to 'block', it's now part of prev.
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    block_header_t *block = (block_header_t *)p - 1;
    if (!is_valid(p, g_size)) return NULL;
    if (block->magic != ARENA_MAGIC) return NULL;
    if (block->free) return NULL; // Cannot realloc a freed block

    size_t current_size = block->size;
    size_t align = 16;
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    size_t header_size = sizeof(block_header_t);
    size_t needed_block_size = header_size + aligned_n;

    // Case 1: Block is large enough
    if (current_size >= needed_block_size) {
        // Can we shrink? The prompt says "grow into that neighbour".
        // It doesn't explicitly say shrink, but realloc usually shrinks.
        // However, to keep merging simple, let's not shrink unless necessary for correctness?
        // The prompt says: "when the block immediately after p is free and large enough it must grow".
        // It doesn't forbid shrinking. But shrinking creates fragmentation.
        // Let's just leave it if it fits, unless we want to merge with next?
        // If we shrink, we create a new free block at the end.
        // Let's stick to: if it fits, leave it. If it doesn't fit, try to expand.
        // If it doesn't fit and can't expand, do a standard realloc (alloc new, copy, free old).
        
        // Check if we can expand into the next block
        uint8_t *end = (uint8_t *)block + sizeof(block_header_t) + current_size;
        if (end < (uint8_t *)g_buf + g_size) {
            block_header_t *next = (block_header_t *)end;
            if (next->magic == ARENA_MAGIC && next->free) {
                size_t next_size = next->size;
                if (current_size + sizeof(block_header_t) + next_size >= needed_block_size) {
                    // Grow into next block
                    // Remove next block header, add its size to current block
                    // But wait, next might have a free block after it?
                    // We need to merge next's free block with current block.
                    
                    // Merge current + next
                    block->size += sizeof(block_header_t) + next_size;
                    
                    // Now check if the new next (after next) is free and merge that too?
                    // Yes, recursive merge or iterative.
                    uint8_t *new_end = (uint8_t *)block + sizeof(block_header_t) + block->size;
                    if (new_end < (uint8_t *)g_buf + g_size) {
                        block_header_t *after_next = (block_header_t *)new_end;
                        if (after_next->magic == ARENA_MAGIC && after_next->free) {
                            block->size += sizeof(block_header_t) + after_next->size;
                        }
                    }
                    
                    // Preserve content
                    // size to copy is min(old_data_size, new_data_size)
                    // old_data_size was current_size - header
                    // new_data_size is aligned_n
                    size_t copy_size = (current_size > aligned_n) ? aligned_n : (current_size - header_size);
                    // Actually, we just keep the data. The user expects data up to old size.
                    // We just need to ensure we don't overwrite user data if we shrink?
                    // We are growing, so we are safe.
                    
                    return p;
                }
            }
        }
        
        // If we are here, the block fits but we can't expand.
        // Do we shrink? Standard realloc shrinks.
        // If we shrink, we create a free block at the end.
        // Let's shrink if the remaining space is large enough for a block header + min data.
        size_t remaining = current_size - needed_block_size;
        if (remaining >= sizeof(block_header_t) + 1) { // 1 byte min data
            // Split off the end
            uint8_t *split_addr = (uint8_t *)block + sizeof(block_header_t) + needed_block_size;
            block_header_t *new_free = (block_header_t *)split_addr;
            new_free->magic = ARENA_MAGIC;
            new_free->size = remaining - sizeof(block_header_t);
            new_free->free = 1;
            
            // Merge new_free with its successor
            uint8_t *new_free_end = (uint8_t *)new_free + sizeof(block_header_t) + new_free->size;
            if (new_free_end < (uint8_t *)g_buf + g_size) {
                block_header_t *succ = (block_header_t *)new_free_end;
                if (succ->magic == ARENA_MAGIC && succ->free) {
                    new_free->size += sizeof(block_header_t) + succ->size;
                }
            }
            
            block->size = needed_block_size;
            return p;
        }
        
        // If we can't shrink (fragmentation too high) and can't expand, we must move.
    }

    // Case 2: Block is too small and cannot be expanded (or no free next block)
    // Standard realloc behavior: alloc new, copy, free old.
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    size_t copy_size = (block->size > aligned_n) ? aligned_n : (block->size - header_size);
    // Actually, copy size is min(old_allocated_data, new_allocated_data).
    // old_allocated_data is block->size - header_size.
    // new_allocated_data is aligned_n.
    size_t actual_copy = block->size - header_size;
    if (actual_copy > aligned_n) {
        actual_copy = aligned_n;
    }
    
    // Copy data
    // Note: p is (block + 1). Data starts at (block + 1).
    // We copy from old p to new p.
    __builtin_memcpy(new_p, p, actual_copy);
    
    // Free old block
    arena_free(p);
    
    return new_p;
}