```c
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))

typedef struct {
    void *start;
    size_t total_size;
    size_t used;
    size_t free_list_offset;
} arena_header_t;

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

static void *arena_ptr_from_header(block_header_t *h) {
    return (char *)h + sizeof(block_header_t);
}

static block_header_t *arena_header_from_ptr(void *p) {
    return (block_header_t *)((char *)p - sizeof(block_header_t));
}

static void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_header_t) + sizeof(block_header_t)) {
        return;
    }

    arena_header_t *header = (arena_header_t *)buf;
    header->start = buf;
    header->total_size = size;
    header->used = sizeof(arena_header_t);
    header->free_list_offset = 0;

    size_t remaining = size - sizeof(arena_header_t);
    size_t aligned_size = ALIGN_UP(sizeof(block_header_t), _Alignof(max_align_t));
    
    if (remaining < aligned_size) {
        return;
    }

    block_header_t *root = (block_header_t *)((char *)buf + sizeof(arena_header_t));
    root->size = remaining - sizeof(block_header_t);
    root->is_free = 1;
    root->next = NULL;
    root->prev = NULL;

    header->free_list_offset = (char *)root - (char *)buf;
}

static void coalesce(arena_header_t *header, block_header_t *block) {
    if (!block->is_free) return;

    block_header_t *next = block->next;
    block_header_t *prev = block->prev;

    if (next && next->is_free) {
        block->size += sizeof(block_header_t) + next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
        next = block->next;
    }

    if (prev && prev->is_free) {
        prev->size += sizeof(block_header_t) + block->size;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
        block = prev;
    }
}

static void unlink_free(arena_header_t *header, block_header_t *block) {
    if (block->prev) {
        block->prev->next = block->next;
    } else {
        header->free_list_offset = (char *)block->next - (char *)header->start;
    }
    if (block->next) {
        block->next->prev = block->prev;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    arena_header_t *header = (arena_header_t *)arena_alloc; // This is a trick to get header, but we need a global or pass it. 
    // Correction: The problem states "All bookkeeping must live inside that buffer". 
    // We cannot have a global pointer to the header because arena_init is called with a buffer.
    // We must assume the caller knows the buffer address or we store the header pointer in the buffer itself.
    // However, the function signature is fixed: void arena_init(void *buf, size_t size).
    // We cannot change the signature. 
    // The standard way for a self-contained arena is to store the header at the beginning of the buffer.
    // But arena_alloc doesn't take the buffer pointer. 
    // This implies the implementation must rely on a static global pointer to the current arena, 
    // OR the problem implies a single arena instance per program.
    // Given "arena_init hands the allocator a buffer", and "arena_alloc" has no buffer arg, 
    // we MUST use a static variable to store the current arena header.
    
    // Re-evaluating constraints: "All bookkeeping must live inside that buffer". 
    // This usually means the header is inside the buffer. 
    // But how does arena_alloc find the buffer? 
    // It must be a global state.
    
    // Let's assume a single arena instance is active at a time.
    static arena_header_t *g_header = NULL;
    
    if (!g_header) return NULL;

    size_t aligned_n = ALIGN_UP(n, _Alignof(max_align_t));
    size_t total_needed = sizeof(block_header_t) + aligned_n;

    block_header_t *best = NULL;
    size_t best_size = 0;
    size_t best_offset = 0;

    // Find best fit in free list
    size_t current_offset = g_header->free_list_offset;
    while (current_offset != 0) {
        block_header_t *curr = (block_header_t *)((char *)g_header->start + current_offset);
        if (curr->size >= aligned_n) {
            if (!best || curr->size < best_size) {
                best = curr;
                best_size = curr->size;
                best_offset = current_offset;
            }
            if (curr->size == aligned_n) break; // Exact fit
        }
        current_offset = (char *)curr->next - (char *)g_header->start;
    }

    if (!best) return NULL;

    unlink_free(g_header, best);

    if (best->size > total_needed + sizeof(block_header_t)) {
        // Split block
        size_t new_block_size = best->size - total_needed;
        block_header_t *new_block = (block_header_t *)((char *)best + total_needed);
        new_block->size = new_block_size;
        new_block->is_free = 1;
        new_block->next = best->next;
        new_block->prev = best->prev;
        
        if (best->next) best->next->prev = new_block;
        if (best->prev) best->prev->next = new_block;
        else g_header->free_list_offset = (char *)new_block - (char *)g_header->start;

        best->size = aligned_n;
        best->next = new_block;
        best->prev = NULL; // best is now the start of the used block, but we need to link it back if we want to merge later? 
        // Actually, best is now allocated. We don't need to link it in the free list.
        // But we need to ensure the free list is updated correctly.
        // The split block (new_block) is added to the free list.
        // We already set new_block->next/prev.
        // We need to insert new_block into the free list.
        // Since we unlinked best, we just need to link new_block.
        // If best was the head, new_block becomes head.
        // If best had a prev, new_block takes its place in the list? No, best is gone from list.
        // We need to insert new_block where best was.
        // Let's re-link new_block into the free list properly.
        
        // Re-insert new_block into free list
        if (best->prev) {
            best->prev->next = new_block;
        } else {
            g_header->free_list_offset = (char *)new_block - (char *)g_header->start;
        }
        if (new_block->next) {
            new_block->next->prev = new_block;
        }
    } else {
        // Use entire block
        best->next = NULL;
        best->prev = NULL;
    }

    best->is_free = 0;
    return arena_ptr_from_header(best);
}

void arena_free(void *p) {
    if (!p) return;
    
    static arena_header_t *g_header = NULL;
    if (!g_header) return;

    block_header_t *block = arena_header_from_ptr(p);
    if ((char *)block < (char *)g_header->start || (char *)block >= (char *)g_header->start + g_header->total_size) {
        return; // Invalid pointer
    }

    block->is_free = 1;
    coalesce(g_header, block);
    
    // Add to free list
    block->next = (block_header_t *)((char *)g_header->start + g_header->free_list_offset);
    block->prev = NULL;
    if (block->next) {
        block->next->prev = block;
    }
    g_header->free_list_offset = (char *)block - (char *)g_header->start;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) {
        return arena_alloc(n);
    }

    static arena_header_t *g_header = NULL;
    if (!g_header) return NULL;

    block_header_t *block = arena_header_from_ptr(p);
    if ((char *)block < (char *)g_header->start || (char *)block >= (char *)g_header->start + g_header->total_size) {
        return NULL;
    }

    size_t old_size = block->size;
    size_t aligned_n = ALIGN_UP(n, _Alignof(max_align_t));
    size_t total_needed = sizeof(block_header_t) + aligned_n;

    if (aligned_n <= old_size) {
        // Shrink or same size
        if (aligned_n < old_size) {
            // Split off the excess
            size_t excess = old_size - aligned_n;
            if (excess >= sizeof(block_header_t) + _Alignof(max_align_t)) {
                block_header_t *new_block = (block_header_t *)((char *)block + total_needed);
                new_block->size = excess - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = block->next;
                new_block->prev = block->prev;
                
                if (block->next) block->next->prev = new_block;
                if (block->prev) block->prev->next = new_block;
                else g_header->free_list_offset = (char *)new_block - (char *)g_header->start;

                block->size = aligned_n;
                block->next = new_block;
                block->prev = NULL;
                
                // Add new_block to free list
                if (block->prev) {
                    block->prev->next = new_block;
                } else {
                    g_header->free_list_offset = (char *)new_block - (char *)g_header->start;
                }
                if (new_block->next) {
                    new_block->next->prev = new_block;
                }
            }
        }
        return p;
    }

    // Need to grow
    block_header_t *next = block->next;
    if (next && next->is_free) {
        size_t combined = old_size + sizeof(block_header_t) + next->size;
        if (combined >= total_needed) {
            // Grow into next
            block->size = aligned_n;
            block->next = next->next;
            if (next->next) {
                next->next->prev = block;
            }
            // Remove next from free list
            unlink_free(g_header, next);
            // Coalesce if next was merged with something else? 
            // We just consumed next. If next had a free neighbor after it, we should merge?
            // The coalesce logic handles merging free blocks. 
            // Since we consumed next, we don't need to merge it.
            // But we might have created a gap if we didn't consume all of next?
            // We consumed enough to satisfy n. If there is leftover in next, we split.
            if (combined > total_needed) {
                size_t excess = combined - total_needed;
                block_header_t *new_block = (block_header_t *)((char *)block + total_needed);
                new_block->size = excess - sizeof(block_header_t);
                new_block->is_free = 1;
                new_block->next = block->next;
                new_block->prev = block->prev;
                
                if (block->next) block->next->prev = new_block;
                if (block->prev) block->prev->next = new_block;
                else g_header->free_list_offset = (char *)new_block - (char *)g_header->start;

                block->size = aligned_n;
                block->next = new_block;
                block->prev = NULL;
                
                // Add new_block to free list
                if (block->prev) {
                    block->prev->next = new_block;
                } else {
                    g_header->free_list_offset = (char *)new_block - (char *)g_header->start;
                }
                if (new_block->next) {
                    new_block->next->prev = new_block;
                }
            }
            return p;
        }
    }

    // Cannot grow in place, allocate new and copy
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;

    size_t copy_size = (n < old_size) ? n : old_size;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}

// Fix: The static variable g_header needs to be set by arena_init.
// We need to modify arena_init to set the global pointer.
// But we cannot change the signature of arena_init.
// We can use a static variable inside arena_init to store the header.
// However, the functions arena_alloc, arena_free, arena_realloc need access to it.
// We will use a static variable in the file scope.

static arena_header_t *g_arena_header = NULL;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(arena_header_t) + sizeof(block_header_t)) {
        g_arena_header = NULL;
        return;
    }

    arena_header_t *header = (arena_header_t *)buf;
    header->start = buf;
    header->total_size = size;
    header->used = sizeof(arena_header_t);
    header->free_list_offset = 0;

    size_t remaining = size - sizeof(arena_header_t);
    size_t aligned_size = ALIGN_UP(sizeof(block_header_t), _Alignof(max_align_t));
    
    if (remaining < aligned_size) {
        g_arena_header = NULL;
        return;
    }

    block_header_t *root = (block_header_t *)((char *)buf + sizeof(arena_header_t));
    root->size = remaining - sizeof(block_header_t);
    root->is_free = 1;
    root->next = NULL;
    root->prev = NULL;

    header->free_list_offset = (char *)root - (char *)buf;
    g_arena_header = header;
}

// Re-implement arena_alloc, arena_free, arena_realloc to use g_arena_header
void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (!g_arena_header) return NULL;

    size_t aligned_n = ALIGN_UP(n, _Alignof(max_align_t));
    size_t total_needed = sizeof(block_header_t) + aligned_n;

    block_header_t *best = NULL;
    size_t best_size = 0;
    size_t best_offset = 0;

    size_t current_offset = g_arena_header->free_list_offset;
    while (current_offset != 0) {
        block_header_t *curr = (block_header_t *)((char *)g_arena_header->start + current_offset);
        if (curr->size >= aligned_n) {
            if (!best || curr->size < best_size) {
                best = curr;
                best_size = curr->size;
                best_offset = current_offset;
            }
            if (curr->size == aligned_n) break;
        }
        current_offset = (char *)curr->next - (char *)g_arena_header->start;
    }

    if (!best) return NULL;

    unlink_free(g_arena_header, best);

    if (best->size > total_needed + sizeof(block_header_t)) {
        size_t new_block_size = best->size - total_needed;
        block_header_t *new_block = (block_header_t *)((char *)best + total_needed);
        new_block->size = new_block_size;
        new_block->is_free = 1;
        new_block->next = best->next;
        new_block->prev = best->prev;
        
        if (best->next) best->next->prev = new_block;
        if (best->prev) best->prev->next = new_block;
        else g_arena_header->free_list_offset = (char *)new_block - (char *)g_arena_header->start;

        best->size = aligned_n;
        best->next = new_block;
        best->prev = NULL;
    } else {
        best->next = NULL;
        best->prev = NULL;
    }

    best->is_free = 0;
    return arena_ptr_from_header(best);
}

void arena_free(void *p) {
    if (!p) return;
    if (!g_arena_header) return;

    block_header_t *block = arena_header_from_ptr(p);
    if ((char *)block < (char *)g_arena_header->start || (char *)block >= (char *)g_arena_header->start + g_arena_header->total_size) {
        return;
    }

    block->is_free = 1;
    coalesce(g_arena_header, block);
    
    block->next = (block