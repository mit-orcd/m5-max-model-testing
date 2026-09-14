#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 
 * Fixed-buffer arena allocator implementation.
 * All bookkeeping is stored inside the provided buffer.
 */

#ifndef ALIGNMENT
#define ALIGNMENT _Alignof(max_align_t)
#endif

/* Align size up to ALIGNMENT */
static inline size_t align_up(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

/* Magic number to identify valid headers */
#define BLOCK_MAGIC 0xDEADBEEF

/* Block header stored before each block */
struct block_header {
    uint32_t magic;
    uint32_t size;      /* Total size of block (header + payload) */
    uint8_t  free;      /* 1 if free, 0 if allocated */
    uint8_t  next_free; /* 1 if next block is free (for fast merge check) */
    uint8_t  prev_free; /* 1 if prev block is free (for fast merge check) */
    uint8_t  padding;
};

/* Helper to get pointer from header */
static inline void *block_to_ptr(const struct block_header *h) {
    return (void *)(h + 1);
}

/* Helper to get header from pointer */
static inline struct block_header *ptr_to_block(void *p) {
    return (struct block_header *)p - 1;
}

/* Global state, but initialized by arena_init */
static void *arena_base = NULL;
static size_t arena_size = 0;
static struct block_header *arena_end = NULL;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(struct block_header) + ALIGNMENT) {
        arena_base = NULL;
        arena_size = 0;
        arena_end = NULL;
        return;
    }

    arena_base = buf;
    arena_size = size;

    /* First block header at the very beginning */
    struct block_header *first = (struct block_header *)buf;

    first->magic = BLOCK_MAGIC;
    first->size = (size_t)((char *)arena_end - (char *)first);
    /* We need to calculate the available space for the first block */
    /* Actually, we just set the size to the total buffer size minus header */
    /* But to be consistent with the linked list logic, we treat the whole buffer as one free block */
    first->size = arena_size - sizeof(struct block_header);
    first->free = 1;
    first->next_free = 0;
    first->prev_free = 0;

    /* Calculate the end pointer */
    arena_end = (struct block_header *)((char *)buf + arena_size);
}

static struct block_header *find_free_block(size_t req_size) {
    struct block_header *curr = (struct block_header *)arena_base;
    size_t aligned_req = align_up(req_size);

    while ((char *)curr < (char *)arena_end) {
        if ((char *)curr + curr->size > (char *)arena_end) {
            /* Corrupted block size, stop */
            break;
        }

        if (curr->free && curr->size >= aligned_req) {
            return curr;
        }

        /* Move to next block */
        curr = (struct block_header *)((char *)curr + curr->size + sizeof(struct block_header));
    }
    return NULL;
}

static void update_neighbors(struct block_header *b, int new_free_status) {
    struct block_header *prev = NULL;
    struct block_header *next = NULL;

    /* Find previous block */
    struct block_header *curr = (struct block_header *)arena_base;
    while ((char *)curr < (char *)b) {
        if ((char *)curr + curr->size + sizeof(struct block_header) == (char *)b) {
            prev = curr;
            break;
        }
        curr = (struct block_header *)((char *)curr + curr->size + sizeof(struct block_header));
    }

    /* Find next block */
    curr = (struct block_header *)((char *)b + b->size + sizeof(struct block_header));
    if ((char *)curr < (char *)arena_end) {
        next = curr;
    }

    if (prev) {
        prev->next_free = new_free_status;
    }
    if (next) {
        next->prev_free = new_free_status;
    }
}

static void merge_free_blocks(struct block_header *b) {
    struct block_header *next = (struct block_header *)((char *)b + b->size + sizeof(struct block_header));
    struct block_header *prev = NULL;

    /* Find previous block */
    struct block_header *curr = (struct block_header *)arena_base;
    while ((char *)curr < (char *)b) {
        if ((char *)curr + curr->size + sizeof(struct block_header) == (char *)b) {
            prev = curr;
            break;
        }
        curr = (struct block_header *)((char *)curr + curr->size + sizeof(struct block_header));
    }

    /* Merge with next if free */
    if (next && next->free && (char *)next + next->size <= (char *)arena_end) {
        b->size += sizeof(struct block_header) + next->size;
        /* Invalidate next block magic to prevent double merge */
        next->magic = 0;
        /* Update prev's next_free status if necessary */
        if (prev) {
            prev->next_free = 1;
        }
        return;
    }

    /* Merge with prev if free */
    if (prev && prev->free) {
        prev->size += sizeof(struct block_header) + b->size;
        /* Invalidate current block magic */
        b->magic = 0;
        /* Update next's prev_free status if necessary */
        if (next) {
            next->prev_free = 1;
        }
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (!arena_base) return NULL;

    size_t aligned_n = align_up(n);
    struct block_header *block = find_free_block(aligned_n);

    if (!block) return NULL;

    /* Split the block if there is enough room for a new header + minimum payload */
    size_t remaining = block->size - aligned_n;
    if (remaining >= sizeof(struct block_header) + ALIGNMENT) {
        /* Create a new free block after the allocated one */
        struct block_header *new_block = (struct block_header *)((char *)block + sizeof(struct block_header) + aligned_n);
        new_block->magic = BLOCK_MAGIC;
        new_block->size = remaining - sizeof(struct block_header);
        new_block->free = 1;
        new_block->next_free = 0;
        new_block->prev_free = 0;

        block->size = aligned_n;
        block->free = 0;
        
        /* Update neighbors */
        update_neighbors(block, 0);
        /* The new block is the next of the current block */
        /* We need to update the 'next' pointer of the block we just split from? 
           Actually, our list is implicit by size, so we just need to ensure consistency */
        /* The 'new_block' is now the 'next' block for 'block' */
        /* We need to update 'block' neighbors (which is 'new_block' now) */
        /* Since we are creating 'new_block' right after 'block', we don't need to do much 
           except ensure the block we return has correct flags. */
    } else {
        block->free = 0;
        update_neighbors(block, 0);
    }

    return block_to_ptr(block);
}

void arena_free(void *p) {
    if (!p || !arena_base) return;
    if (p < arena_base || (char *)p >= (char *)arena_end) return;

    struct block_header *block = ptr_to_block(p);
    if (block->magic != BLOCK_MAGIC) return; /* Invalid pointer or not managed by this arena */

    block->free = 1;
    update_neighbors(block, 1);
    merge_free_blocks(block);
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) {
        return arena_alloc(n);
    }
    if (!arena_base) return NULL;

    struct block_header *block = ptr_to_block(p);
    if (block->magic != BLOCK_MAGIC) return NULL;

    size_t aligned_n = align_up(n);
    size_t current_size = block->size;

    /* If we need less or equal space, just return the same pointer */
    if (aligned_n <= current_size) {
        return p;
    }

    /* Check if we can grow into the next block */
    struct block_header *next = (struct block_header *)((char *)block + block->size + sizeof(struct block_header));
    
    /* Verify next block is within bounds */
    if ((char *)next + sizeof(struct block_header) >= (char *)arena_end) {
        next = NULL;
    }

    if (next && next->free) {
        /* Check if next block is large enough to cover the difference */
        size_t needed = aligned_n - current_size;
        if (next->size >= needed) {
            /* Grow current block into next */
            block->size = aligned_n;
            /* Shrink next block */
            next->size -= needed;
            next = (struct block_header *)((char *)next + needed);
            /* If next block became too small to hold a header, we might need to merge or just leave it?
               Actually, we just reduced the size. If it's still valid, keep it.
               But wait, we moved the pointer of 'next' forward. 
               We need to update the 'next' block's header to reflect the new position?
               No, the 'next' variable is a pointer to the old header location.
               The actual next block header is now at `next_old + needed`.
               Wait, if we grow into the next block, we are consuming part of the next block's space.
               The next block's header moves.
            */
            
            /* Let's recalculate the new next block pointer */
            struct block_header *old_next = (struct block_header *)((char *)block + block->size + sizeof(struct block_header));
            /* Wait, block->size has changed. 
               The next block starts at (char *)block + block->size + sizeof(struct block_header).
               But we are consuming space.
               Let's rethink: 
               Old block ends at X. Next block starts at X + sizeof(header).
               We want to grow block by 'needed'.
               The new block ends at X + needed.
               The next block header must move forward by 'needed'.
               The size of the next block decreases by 'needed'.
            */
            
            /* We need to shift the next block header */
            struct block_header *new_next = (struct block_header *)((char *)old_next + needed);
            memcpy(new_next, old_next, sizeof(struct block_header));
            new_next->size = old_next->size - needed;
            /* If the remaining size is too small to be a valid block (less than header size?), 
               we might want to merge it or mark it invalid? 
               For simplicity, if it's too small, we can just merge it into the current block or free list.
               But the spec says "grow into that neighbour".
               Let's assume the remaining space is still valid or we merge it.
               If remaining < sizeof(header), we can't have a header there. 
               So if remaining < sizeof(header), we just extend fully and the next block disappears (merged).
            */
            
            if (new_next->size < sizeof(struct block_header)) {
                /* Not enough space for a header, merge completely */
                block->size += new_next->size + sizeof(struct block_header);
                new_next->magic = 0; /* Invalidate */
            } else {
                /* Keep the next block but shifted */
                /* Update neighbors flags */
                /* block is still allocated, so next_free of prev is 0 */
                /* next is still free, so prev_free of next is 1 */
                /* Update the 'next' block's prev_free status if needed? 
                   It was free, so prev_free was 1. It remains 1.
                   The block we grew is still allocated.
                */
            }
            
            /* Update neighbors links if necessary */
            /* The 'next' block pointer in the implicit list is now 'new_next' */
            /* We don't have explicit pointers, so we rely on size traversal. 
               The traversal will naturally find the shifted header.
               However, we need to ensure the 'next' block's prev_free flag is correct relative to its new position.
               The block before 'new_next' is 'block' (allocated). So prev_free should be 0.
            */
            new_next->prev_free = 0;
            
            /* Also update the block before 'block' if it was free, to point to the new 'next' block?
               No, the traversal finds 'new_next' by size.
               But we need to make sure the neighbor updates are consistent.
            */
            
            /* We need to update the 'prev' of 'new_next' (which is 'block' now) */
            /* And the 'next' of 'block' (which is 'new_next') */
            /* But we don't store 'next' pointer in the header. We rely on the next block knowing its prev is free?
               Wait, the 'next_free' and 'prev_free' flags are for optimization.
               We need to update the 'prev' of 'new_next' (which is 'block')? No, 'block' is allocated.
               We need to update the 'next' of the block before 'block' if it exists.
            */
            
            struct block_header *prev = NULL;
            curr = (struct block_header *)arena_base;
            while ((char *)curr < (char *)block) {
                if ((char *)curr + curr->size + sizeof(struct block_header) == (char *)block) {
                    prev = curr;
                    break;
                }
                curr = (struct block_header *)((char *)curr + curr->size + sizeof(struct block_header));
            }
            if (prev) {
                prev->next_free = 0; /* block is still allocated */
            }
            
            /* If we didn't fully consume the next block, we need to ensure the new next block is valid */
            if (new_next->magic == BLOCK_MAGIC) {
                /* Update its prev_free to 0 because block is allocated */
                new_next->prev_free = 0;
            } else {
                /* If we merged completely, the next block after 'new_next' (if any) needs update */
                struct block_header *new_next_next = (struct block_header *)((char *)new_next + new_next->size + sizeof(struct block_header));
                if ((char *)new_next_next < (char *)arena_end && new_next_next->magic == BLOCK_MAGIC) {
                    new_next_next->prev_free = 0;
                }
            }
            
            return p;
        }
    }

    /* Cannot grow in place, need to allocate new block and copy */
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;

    size_t copy_size = (n < current_size) ? n : current_size;
    memcpy(new_p, p, copy_size);

    arena_free(p);
    return new_p;
}