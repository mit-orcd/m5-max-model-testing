#include <stddef.h>
#include <stdalign.h>

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header;

static block_header *head;
static size_t buffer_size;
static char *buffer_start;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(block_header)) {
        head = NULL;
        return;
    }
    
    buffer_start = (char *)buf;
    buffer_size = size;
    
    block_header *first = (block_header *)buffer_start;
    first->size = size - sizeof(block_header);
    first->is_free = 1;
    first->next = NULL;
    first->prev = NULL;
    
    head = first;
}

static block_header *align_ptr(void *p) {
    size_t align = _Alignof(max_align_t);
    size_t addr = (size_t)p;
    size_t offset = (align - (addr % align)) % align;
    return (block_header *)((char *)p + offset);
}

static void split_block(block_header *block, size_t new_size) {
    size_t available = block->size - new_size;
    if (available < sizeof(block_header)) return;
    
    char *block_addr = (char *)block;
    block_header *new_block = (block_header *)(block_addr + sizeof(block_header) + new_size);
    
    new_block->size = available - sizeof(block_header);
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    }
    
    block->next = new_block;
    block->size = new_size;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    block_header *block = head;
    while (block) {
        if (block->is_free) {
            size_t required = n;
            size_t header_size = sizeof(block_header);
            size_t total_needed = header_size + required;
            
            if (block->size >= total_needed) {
                size_t align = _Alignof(max_align_t);
                size_t current_addr = (size_t)(block + 1);
                size_t offset = (align - (current_addr % align)) % align;
                
                if (offset > 0 && (block->size - offset >= total_needed)) {
                    total_needed += offset;
                }
                
                if (block->size >= total_needed) {
                    block->is_free = 0;
                    if (block->size - total_needed > sizeof(block_header)) {
                        split_block(block, block->size - sizeof(block_header) - (block->size - total_needed));
                        // Adjust split to account for alignment if needed, simplified here by assuming no split needed or handled
                        // Actually, split_block logic needs to be robust. Let's rely on the fact that we can just use the whole block if splitting is complex.
                        // For simplicity in this strict allocator, we will just use the block as is or split if strictly necessary without alignment padding in split logic unless we manage it.
                        // Re-implementing split to handle alignment padding inside the split logic:
                        // We need to ensure the *returned* data is aligned.
                        // The header is always at the base of the block.
                        // The user data is at (block + 1) + padding.
                        // The remaining space for a new block starts after user data + padding.
                    }
                    return (void*)((char *)block + header_size + ((block->size - header_size) % align > 0 ? (align - ((block->size - header_size) % align)) : 0));
                }
            }
        }
        block = block->next;
    }
    return NULL;
}

// Refined alloc to handle alignment correctly
void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t align = _Alignof(max_align_t);
    block_header *block = head;
    
    while (block) {
        if (block->is_free) {
            size_t header_size = sizeof(block_header);
            size_t base_data_addr = (size_t)(block + 1);
            size_t padding = (align - (base_data_addr % align)) % align;
            size_t total_block_needed = header_size + padding + n;
            
            if (block->size >= total_block_needed) {
                block->is_free = 0;
                
                // Try to split if there is enough space for a new header + some data
                size_t remaining = block->size - total_block_needed;
                if (remaining > sizeof(block_header)) {
                    block_header *new_block = (block_header *)((char *)block + header_size + padding + n);
                    new_block->size = remaining - sizeof(block_header);
                    new_block->is_free = 1;
                    new_block->next = block->next;
                    new_block->prev = block;
                    
                    if (block->next) {
                        block->next->prev = new_block;
                    }
                    block->next = new_block;
                } else {
                    block->next = NULL;
                    if(block->prev) block->prev->next = NULL;
                    // If this was head, update head
                    if (head == block) {
                        head = NULL;
                    }
                }
                
                return (void*)((char *)block + header_size + padding);
            }
        }
        block = block->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    block_header *block = (block_header *)((char *)p - sizeof(block_header));
    if (block->is_free) return; // Already free
    
    block->is_free = 1;
    
    // Merge with next if free
    if (block->next && block->next->is_free) {
        block_header *next_block = block->next;
        block->size += sizeof(block_header) + next_block->size;
        block->next = next_block->next;
        if (next_block->next) {
            next_block->next->prev = block;
        }
    }
    
    // Merge with prev if free
    if (block->prev && block->prev->is_free) {
        block_header *prev_block = block->prev;
        prev_block->size += sizeof(block_header) + block->size;
        prev_block->next = block->next;
        if (block->next) {
            block->next->prev = prev_block;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);
    
    block_header *block = (block_header *)((char *)p - sizeof(block_header));
    size_t old_size = block->size;
    
    // Check if we can grow into the next block
    if (!block->next || !block->next->is_free) {
        // Cannot grow in place, need to allocate new block
        void *new_p = arena_alloc(n);
        if (!new_p) return NULL;
        size_t copy_size = (old_size < n) ? old_size : n;
        // Note: old_size here includes alignment padding, so we must be careful.
        // The actual user data size was (old_size - header - padding).
        // We need to calculate the actual user size.
        size_t align = _Alignof(max_align_t);
        size_t padding = (align - ((size_t)(block + 1) % align)) % align;
        size_t old_user_size = old_size - sizeof(block_header) - padding;
        size_t copy_len = (old_user_size < n) ? old_user_size : n;
        
        if (copy_len > 0) {
            __builtin_memcpy(new_p, p, copy_len);
        }
        arena_free(p);
        return new_p;
    }
    
    block_header *next_block = block->next;
    size_t next_size = next_block->size;
    size_t available_growth = next_size; // The next block is free, we can take its space
    
    // We need to accommodate n in the current block's data area + next block's data area
    // But we need to preserve alignment.
    // Let's simplify: if we can fit n into the current block's total size (including next block), do it.
    // Since next block is free and aligned, we can just merge headers.
    // The user data is at (block + 1 + padding).
    // The next block starts at (block + 1 + padding + old_user_size).
    // Wait, the next block header is at (block + 1 + padding + old_user_size).
    // If we merge, the new block header size is old_size + sizeof(next_block_header) + next_block_size? No.
    // The next block's header is right after the current block's user data.
    // Current block: [Header | Padding | User Data | (Next Block Header)]
    // Next block: [Header | User Data]
    
    // To merge, we effectively extend the current block's size to include the next block's header and data.
    // But we need to ensure the *new* user data area is still aligned for the requested size.
    // Since we are merging a free block, the next block's header is aligned (as it was part of the list).
    // The current user data area ends at (block + 1 + padding + old_user_size).
    // The next block header starts exactly there.
    // So the merged block will have: [Current Header | Padding | Old User | Next Header | Next User]
    // The user data for the new request starts at (block + 1 + padding).
    // The max size available is old_user_size + next_block->size.
    // However, we must ensure that the *new* user data area (for n) is valid.
    // Since the next block was free, its header is placed at a boundary.
    // If we merge, the new block's header is the current block's header.
    // The user data for the new request is at (block + 1 + padding).
    // The total size available for user data is (block->size + next_block->size - sizeof(block_header) - padding).
    // Wait, block->size is the current block's total size (excluding its own header? No, usually total).
    // In my implementation, block->size is the total size of the block including the header? 
    // No, in split_block, I did: new_block->size = available - sizeof(block_header).
    // And block->size = new_size (user data size + padding? No).
    // Let's re-evaluate the size definition.
    // In arena_init: first->size = size - sizeof(block_header). This is the usable space + alignment padding?
    // No, it's the space available for data + alignment padding?
    // Actually, let's define block->size as the total size of the block including the header?
    // No, the code in split_block: block->size = new_size.
    // And arena_alloc calculates: total_block_needed = header_size + padding + n.
    // So block->size should be the total size of the block including the header?
    // If block->size = 100, header=16, padding=4, n=80. 16+4+80 = 100.
    // So block->size is the total size of the block (header + data).
    
    // If we merge:
    // Current block: size S1 (includes header).
    // Next block: size S2 (includes header).
    // Merged block size = S1 + S2.
    // But we need to check if the new request n can fit in the merged block's user area.
    // The user area of the merged block starts at (block + 1 + padding).
    // The max user size is (S1 + S2) - sizeof(block_header) - padding.
    // Wait, the next block's padding is handled by its own header alignment?
    // Yes, next_block->size includes its own header.
    // So the merged block has: [Current Header | Padding | Old User | Next Header | Next User].
    // The user data for the new request is at (block + 1 + padding).
    // The total user space available is (S1 - sizeof(block_header) - padding) + (S2 - sizeof(block_header)).
    // Wait, S2 includes the next header. So the space for new user data is:
    // (S1 - sizeof(block_header) - padding) + (S2 - sizeof(block_header)).
    // But we must ensure that the new request n fits in this space.
    // Also, we must ensure that the new request n does not require a new padding (which it won't, as we are extending).
    // But what if the old user data was not fully used?
    // The request n might be larger than the old user data.
    // We need to check if (S1 + S2 - 2 * sizeof(block_header) - padding) >= n.
    // If so, we can merge.
    
    // However, there is a catch: the next block might have been allocated with a specific alignment.
    // When we merge, the combined block's user area starts at (block + 1 + padding).
    // This alignment is fixed.
    // If n requires a larger alignment than max_align_t, it fails (but we assume max_align_t).
    // The only issue is if the next block's header is not aligned relative to the start of the merged block's user area?
    // No, the next block's header is at (block + 1 + padding + old_user_size).
    // The next block's header is always aligned to max_align_t (because it was created as a block).
    // So the merged block is valid.
    
    size_t align = _Alignof(max_align_t);
    size_t padding = (align - ((size_t)(block + 1) % align)) % align;
    size_t old_user_size = block->size - sizeof(block_header) - padding;
    size_t next_user_size = next_block->size - sizeof(block_header);
    size_t total_user_size = old_user_size + next_user_size;
    
    if (total_user_size >= n) {
        // Merge
        block->size += next_block->size;
        block->next = next_block->next;
        if (next_block->next) {
            next_block->next->prev = block;
        }
        return p;
    }
    
    // Cannot merge, allocate new
    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;
    size_t copy_len = (old_user_size < n) ? old_user_size : n;
    if (copy_len > 0) {
        __builtin_memcpy(new_p, p, copy_len);
    }
    arena_free(p);
    return new_p;
}