#include <stddef.h>
#include <stdint.h>

#define ARENA_MAGIC 0xA110CA7E

typedef struct alignas(max_align_t) block_header {
    size_t size;
    uint32_t magic;
    uint32_t is_free;
    struct block_header *next;
} block_header_t;

static void *g_buf = NULL;
static size_t g_buf_size = 0;
static block_header_t *g_head = NULL;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_buf_size = size;
    
    // Ensure minimum alignment for the first block
    uintptr_t addr = (uintptr_t)buf;
    size_t align_mod = addr % __alignof__(block_header_t);
    if (align_mod) {
        addr += __alignof__(block_header_t) - align_mod;
    }
    
    size_t effective_size = size - (addr - (uintptr_t)buf);
    if (effective_size < sizeof(block_header_t)) {
        g_head = NULL;
        return;
    }

    g_head = (block_header_t *)addr;
    g_head->size = effective_size - sizeof(block_header_t);
    g_head->magic = ARENA_MAGIC;
    g_head->is_free = 1;
    g_head->next = NULL;
}

static block_header_t *get_prev_block(block_header_t *b) {
    char *start = (char *)g_buf;
    char *current = (char *)b;
    char *ptr = start;
    
    while (ptr < current) {
        block_header_t *prev = (block_header_t *)ptr;
        if (prev->magic != ARENA_MAGIC) break;
        if ((char *)prev + sizeof(block_header_t) + prev->size == current) {
            return prev;
        }
        ptr += sizeof(block_header_t) + prev->size;
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_head == NULL) return NULL;
    
    // Align request size up to max_align_t
    size_t align = __alignof__(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    size_t total_needed = sizeof(block_header_t) + aligned_n;
    
    block_header_t *curr = g_head;
    block_header_t *prev = NULL;
    
    while (curr != NULL) {
        if (curr->is_free && curr->size >= aligned_n) {
            // Found a suitable block
            size_t remaining = curr->size - aligned_n;
            
            if (remaining > sizeof(block_header_t) + align) {
                // Split the block
                block_header_t *new_block = (block_header_t *)((char *)curr + sizeof(block_header_t) + aligned_n);
                new_block->size = remaining - sizeof(block_header_t);
                new_block->magic = ARENA_MAGIC;
                new_block->is_free = 1;
                new_block->next = curr->next;
                
                curr->size = aligned_n;
                curr->is_free = 0;
                curr->next = new_block;
            } else {
                // Take the whole block, merge with next if free
                curr->is_free = 0;
                if (curr->next && curr->next->is_free) {
                    curr->size += sizeof(block_header_t) + curr->next->size;
                    curr->next = curr->next->next;
                }
            }
            
            return (void *)((char *)curr + sizeof(block_header_t));
        }
        
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    block_header_t *block = (block_header_t *)((char *)p - sizeof(block_header_t));
    
    if (block->magic != ARENA_MAGIC || block->is_free) return;
    
    block->is_free = 1;
    
    // Merge with next block if free
    if (block->next && block->next->is_free) {
        block->size += sizeof(block_header_t) + block->next->size;
        block->next = block->next->next;
    }
    
    // Merge with previous block if free
    block_header_t *prev = get_prev_block(block);
    if (prev && prev->is_free) {
        prev->size += sizeof(block_header_t) + block->size;
        prev->next = block->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header_t *block = (block_header_t *)((char *)p - sizeof(block_header_t));
    
    if (block->magic != ARENA_MAGIC || !block->is_free) {
        size_t old_size = block->size;
        size_t align = __alignof__(max_align_t);
        size_t aligned_n = (n + align - 1) & ~(align - 1);
        
        // Try to grow into the next block if it's free
        if (block->next && block->next->is_free) {
            size_t new_total_size = sizeof(block_header_t) + block->size + sizeof(block_header_t) + block->next->size;
            size_t needed = sizeof(block_header_t) + aligned_n;
            
            if (new_total_size >= needed) {
                // Grow in place
                block->size = new_total_size - sizeof(block_header_t);
                
                // If still have excess, split
                size_t remaining = block->size - aligned_n;
                if (remaining > sizeof(block_header_t) + align) {
                    block_header_t *new_block = (block_header_t *)((char *)block + sizeof(block_header_t) + aligned_n);
                    new_block->size = remaining - sizeof(block_header_t);
                    new_block->magic = ARENA_MAGIC;
                    new_block->is_free = 1;
                    new_block->next = block->next->next;
                    block->size = aligned_n;
                    block->next = new_block;
                } else {
                    // Take everything including next block
                    if (block->next->next) {
                        block->next = block->next->next;
                    } else {
                        block->next = NULL;
                    }
                }
                
                return p;
            }
        }
        
        // Cannot grow in place, allocate new
        void *new_p = arena_alloc(n);
        if (new_p != NULL) {
            size_t copy_size = (old_size < aligned_n) ? old_size : aligned_n;
            // Preserve contents up to min(old_size, new_size)
            // Note: old_size is the usable size, aligned_n is aligned new size
            size_t actual_copy = old_size;
            if (actual_copy > n) actual_copy = n;
            // Actually, we should copy up to min(old_usable, new_usable)
            // But aligned_n might be larger than n due to alignment
            // The spec says "Contents up to the smaller of the old and new sizes are preserved"
            // "new sizes" refers to the requested size n
            size_t to_copy = old_size < n ? old_size : n;
            if (to_copy > 0) {
                __builtin_memcpy(new_p, p, to_copy);
            }
            arena_free(p);
            return new_p;
        }
    }
    
    return NULL;
}