#include <stddef.h>
#include <stdint.h>

typedef struct block_header {
    size_t size;
    int is_free;
} block_header;

static void *g_buf;
static size_t g_size;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
}

static _Alignof(max_align_t) _Alignas(max_align_t) char padding[0];

#define ALIGNMENT _Alignof(max_align_t)
#define HEADER_SIZE ((sizeof(block_header) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define BLOCK_SIZE(hdr) ((hdr)->size)
#define SET_BLOCK_SIZE(hdr, s) ((hdr)->size = (s))
#define IS_FREE(hdr) ((hdr)->is_free)
#define SET_FREE(hdr, v) ((hdr)->is_free = (v))

#define PTR_OFFSET(ptr, bytes) ((void *)((char *)(ptr) + (bytes)))
#define PTR_DIFF(p1, p2) ((size_t)((char *)(p1) - (char *)(p2)))

static block_header *get_header(void *p) {
    return (block_header *)PTR_OFFSET(p, -HEADER_SIZE);
}

static void *get_block_ptr(block_header *hdr) {
    return PTR_OFFSET(hdr, HEADER_SIZE);
}

static block_header *get_next_block(void *p) {
    block_header *current = get_header(p);
    size_t current_data_size = BLOCK_SIZE(current);
    return (block_header *)PTR_OFFSET(current, HEADER_SIZE + current_data_size);
}

static int is_end_of_buffer(void *p) {
    return p >= PTR_OFFSET(g_buf, g_size);
}

static block_header *find_free_block(size_t n) {
    block_header *current = (block_header *)g_buf;
    size_t remaining = g_size;

    while (!is_end_of_buffer(current)) {
        size_t header_size = HEADER_SIZE;
        size_t block_size = BLOCK_SIZE(current);
        
        if (block_size == 0) break;

        if (IS_FREE(current)) {
            // Check if we can fit the request with minimal waste
            // We need at least n bytes of data.
            if (block_size >= n) {
                return current;
            }
        }

        current = (block_header *)PTR_OFFSET(current, header_size + block_size);
        if (is_end_of_buffer(current)) break;
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Align request size to alignment boundary
    size_t aligned_n = (n + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
    
    // Find a free block
    block_header *free_block = find_free_block(aligned_n);
    
    if (free_block == NULL) return NULL;
    
    size_t current_block_size = BLOCK_SIZE(free_block);
    
    // If the free block is significantly larger, split it
    // Minimum split size: HEADER_SIZE + min allocation (aligned)
    size_t min_split_size = HEADER_SIZE + aligned_n;
    
    if (current_block_size >= min_split_size + aligned_n) {
        // Split the block
        size_t new_block_size = current_block_size - aligned_n;
        
        // Keep the current block as the new allocated block?
        // No, usually we take the beginning for the new block, and leave the rest free.
        // Or we take the end. Let's take the beginning.
        
        // Update current block to be allocated
        SET_BLOCK_SIZE(free_block, aligned_n);
        SET_FREE(free_block, 0);
        
        // Create new free block at the end
        block_header *new_free_block = (block_header *)PTR_OFFSET(free_block, HEADER_SIZE + aligned_n);
        SET_BLOCK_SIZE(new_free_block, new_block_size);
        SET_FREE(new_free_block, 1);
        
        return get_block_ptr(free_block);
    } else {
        // Take the whole block
        SET_FREE(free_block, 0);
        return get_block_ptr(free_block);
    }
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    // Check if p is within the buffer
    if (p < g_buf || p >= PTR_OFFSET(g_buf, g_size)) return;
    
    block_header *hdr = get_header(p);
    SET_FREE(hdr, 1);
    
    // Merge with next block if free
    block_header *next_hdr = get_next_block(p);
    if (!is_end_of_buffer(next_hdr) && IS_FREE(next_hdr)) {
        size_t next_size = BLOCK_SIZE(next_hdr);
        hdr->size += HEADER_SIZE + next_size;
        // Effectively remove next_hdr from the chain by extending current size
    }
    
    // Merge with previous block if free
    // We need to find the previous block. Since we don't have a linked list,
    // we must scan from the beginning or maintain prev pointers.
    // Scanning from beginning is O(N) but simple.
    block_header *current = (block_header *)g_buf;
    while (!is_end_of_buffer(current)) {
        size_t block_sz = BLOCK_SIZE(current);
        if (block_sz == 0) break;
        
        void *next_block_ptr = PTR_OFFSET(current, HEADER_SIZE + block_sz);
        if (next_block_ptr == p) {
            if (IS_FREE(current)) {
                current->size += HEADER_SIZE + hdr->size;
                // Merged. Done.
                return;
            }
            break; // Found the block before p, and it's not free, so no merge needed with prev
        }
        current = (block_header *)next_block_ptr;
        if (is_end_of_buffer(current)) break;
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
    
    block_header *hdr = get_header(p);
    size_t current_size = BLOCK_SIZE(hdr);
    
    // If shrinking or same size, just return p
    // Note: We don't shrink the block in the arena to keep it simple and avoid fragmentation issues
    // with merging later, unless it's significantly larger. 
    // Standard realloc behavior: if n <= current_size, return p.
    if (n <= current_size) {
        return p;
    }
    
    // Try to expand in place
    block_header *next_hdr = get_next_block(p);
    if (!is_end_of_buffer(next_hdr) && IS_FREE(next_hdr)) {
        size_t next_size = BLOCK_SIZE(next_hdr);
        size_t new_total_size = current_size + HEADER_SIZE + next_size;
        
        if (new_total_size >= n) {
            // Expand into the next block
            // Update current header size
            SET_BLOCK_SIZE(hdr, n); // We only claim the needed space? 
            // Or do we claim the whole merged space? 
            // If we claim the whole space, we can't easily split later if we don't need it all?
            // Actually, if we expand, we should take the whole merged block and then split if there's excess.
            
            // Let's merge the blocks first conceptually, then split.
            // Current block becomes size: current_size + HEADER_SIZE + next_size
            SET_BLOCK_SIZE(hdr, current_size + HEADER_SIZE + next_size);
            SET_FREE(hdr, 0); // Ensure it's marked allocated (it is)
            
            // The next block is effectively removed.
            
            // Now check if we can split off the remainder
            size_t remaining = BLOCK_SIZE(hdr) - HEADER_SIZE; // Total data size of current block
            if (remaining > n) {
                size_t split_size = remaining - n;
                // Current block keeps 'n'
                SET_BLOCK_SIZE(hdr, n);
                
                // Create new free block
                block_header *new_free = (block_header *)PTR_OFFSET(hdr, HEADER_SIZE + n);
                SET_BLOCK_SIZE(new_free, split_size);
                SET_FREE(new_free, 1);
                
                // Merge new_free with its next if free
                block_header *new_next = get_next_block((void*)new_free);
                if (!is_end_of_buffer(new_next) && IS_FREE(new_next)) {
                    size_t new_next_size = BLOCK_SIZE(new_next);
                    new_free->size += HEADER_SIZE + new_next_size;
                }
            }
            
            return p;
        }
    }
    
    // Cannot expand in place, allocate new
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    // Copy old data
    size_t copy_size = (n < current_size) ? n : current_size;
    // Actually, realloc preserves up to min(old_size, new_size)
    copy_size = (current_size < n) ? current_size : n;
    
    // Wait, current_size is the allocated size. The user might have written up to current_size.
    // We should copy min(old_allocated_size, new_requested_size).
    // But wait, what if the user wrote more than n? That's UB in standard realloc, 
    // but we should preserve what we can.
    // Standard: "If the object is resized, its contents up to the minimum of the old and new sizes shall be preserved."
    size_t old_size = current_size;
    if (n < old_size) {
        old_size = n;
    }
    
    // Copy data
    // Note: p is the data pointer. new_p is the new data pointer.
    // We need to copy from p to new_p.
    // But wait, if we are shrinking, we copy min(old, new).
    // If we are growing, we copy old.
    size_t bytes_to_copy = (old_size < n) ? old_size : n;
    
    // However, we need to be careful. If we are shrinking, n is the new size.
    // If we are growing, n is the new size, old_size is the old allocated size.
    // The amount to preserve is min(old_allocated_size, new_requested_size).
    
    // Let's re-evaluate.
    // old_allocated_size = current_size.
    // new_requested_size = n.
    // bytes_to_preserve = (current_size < n) ? current_size : n;
    
    // But wait, if n is larger, we preserve all of current_size.
    // If n is smaller, we preserve n.
    
    // My logic above:
    // if (n <= current_size) return p; -> This handles shrinking.
    // So we are only here if n > current_size.
    // Therefore, bytes_to_preserve = current_size.
    
    // Let's double check.
    // If n > current_size, we allocate new. We copy current_size bytes.
    // Then we free old? No, we don't free old in this function, the caller doesn't free old.
    // But in arena_alloc, we can't free old because we are inside realloc.
    // Actually, standard realloc implementation:
    // 1. Allocate new block.
    // 2. Copy data.
    // 3. Free old block.
    
    // But in an arena, we can't easily "free" the old block if we just allocated a new one
    // because the new one might be anywhere.
    // We must free the old block explicitly after copying.
    
    // Wait, if we are in the "Cannot expand in place" branch:
    // 1. Allocate new block.
    // 2. Copy min(old_size, new_size) bytes.
    // 3. Free old block.
    
    // But wait, if we free the old block, it might merge with neighbors.
    // That's fine.
    
    // However, there is a catch: if arena_alloc fails, we should return NULL and NOT free the old block?
    // Standard realloc: if it fails, the original pointer remains valid.
    // So we must check allocation success before freeing old.
    
    void *temp = new_p;
    size_t copy_len = current_size; // Since n > current_size
    
    // Copy data
    // Use memmove to handle overlap, though unlikely in arena alloc unless we allocated in the same buffer
    // But arena_alloc might allocate in the same buffer if it splits?
    // No, arena_alloc allocates from free blocks. p is from an allocated block.
    // If we allocate a new block, it's likely elsewhere.
    
    // Safe to use memcpy or memmove
    // Since we are in an arena, we know the layout.
    
    // Copy
    for (size_t i = 0; i < copy_len; i++) {
        ((char*)temp)[i] = ((char*)p)[i];
    }
    
    // Free old block
    arena_free(p);
    
    return temp;
}