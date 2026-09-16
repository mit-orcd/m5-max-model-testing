#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

static void *arena_buf = NULL;
static size_t arena_size = 0;

typedef struct BlockHeader {
    size_t size;
    int is_free;
    struct BlockHeader *next;
    struct BlockHeader *prev;
} BlockHeader;

static void *align_ptr(void *p, size_t align) {
    uintptr_t addr = (uintptr_t)p;
    uintptr_t mask = align - 1;
    return (void *)((addr + mask) & ~mask);
}

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    
    if (size < sizeof(BlockHeader)) {
        arena_size = 0;
        return;
    }

    size_t offset = sizeof(BlockHeader);
    void *data_start = align_ptr((char *)buf + offset, _Alignof(max_align_t));
    
    size_t data_end = (char *)buf + size;
    if ((char *)data_start > (char *)data_end) {
        arena_size = 0;
        return;
    }
    
    size_t usable_data_size = (char *)data_end - (char *)data_start;
    
    BlockHeader *header = (BlockHeader *)buf;
    header->size = usable_data_size;
    header->is_free = 1;
    header->next = NULL;
    header->prev = NULL;
}

static BlockHeader *find_free_block(size_t n) {
    BlockHeader *curr = (BlockHeader *)arena_buf;
    while (curr) {
        if (curr->is_free && curr->size >= n) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_size == 0 || arena_buf == NULL) {
        return NULL;
    }

    size_t aligned_n = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    if (aligned_n < n) return NULL; // overflow check

    BlockHeader *block = find_free_block(aligned_n);
    if (!block) return NULL;

    block->is_free = 0;

    size_t header_size = sizeof(BlockHeader);
    size_t min_total = header_size + aligned_n;
    
    if (block->size >= min_total + sizeof(BlockHeader) + _Alignof(max_align_t)) {
        BlockHeader *new_block = (BlockHeader *)((char *)block + header_size + aligned_n);
        new_block->size = block->size - (header_size + aligned_n);
        new_block->is_free = 1;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        block->next = new_block;
        block->size = aligned_n;
    }

    void *data_ptr = (char *)block + sizeof(BlockHeader);
    return align_ptr(data_ptr, _Alignof(max_align_t));
}

void arena_free(void *p) {
    if (p == NULL || arena_buf == NULL) return;

    BlockHeader *header = (BlockHeader *)arena_buf;
    BlockHeader *block = NULL;
    while (header) {
        void *data_ptr = (char *)header + sizeof(BlockHeader);
        void *aligned_ptr = align_ptr(data_ptr, _Alignof(max_align_t));
        if (aligned_ptr == p) {
            block = header;
            break;
        }
        header = header->next;
    }

    if (!block) return;

    block->is_free = 1;

    // Merge with next
    if (block->next && block->next->is_free) {
        BlockHeader *next = block->next;
        block->size += sizeof(BlockHeader) + next->size;
        block->next = next->next;
        if (next->next) {
            next->next->prev = block;
        }
    }

    // Merge with prev
    if (block->prev && block->prev->is_free) {
        BlockHeader *prev = block->prev;
        prev->size += sizeof(BlockHeader) + block->size;
        prev->next = block->next;
        if (block->next) {
            block->next->prev = prev;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (p == NULL) {
        return arena_alloc(n);
    }

    size_t aligned_n = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    if (aligned_n < n) return NULL;

    BlockHeader *block = NULL;
    BlockHeader *header = (BlockHeader *)arena_buf;
    while (header) {
        void *data_ptr = (char *)header + sizeof(BlockHeader);
        void *aligned_ptr = align_ptr(data_ptr, _Alignof(max_align_t));
        if (aligned_ptr == p) {
            block = header;
            break;
        }
        header = header->next;
    }

    if (!block) return NULL;

    size_t header_size = sizeof(BlockHeader);
    size_t current_data_size = block->size;

    if (current_data_size >= aligned_n) {
        // Check if we can split and return extra space, but requirement says 
        // "grow into neighbour" if free and large enough. 
        // If current is big enough, we just leave it (no need to shrink unless we want to).
        // However, standard realloc behavior usually shrinks. 
        // The prompt specifically emphasizes growing into next free block.
        // Let's check if we can merge with next to potentially satisfy a larger request?
        // No, current is already big enough.
        // But if it's *much* bigger, maybe split? The prompt doesn't explicitly require shrinking.
        // Let's just return p.
        
        // Wait, if we have extra space, we might want to split to help future allocs?
        // The prompt says: "when the block immediately after p is free and large enough it must grow".
        // It doesn't say we MUST split if too big. But good allocators do.
        // Let's implement splitting if significantly larger.
        size_t min_req = aligned_n;
        size_t min_header_req = min_req + header_size;
        
        // Check if remaining space can form a valid block
        if (current_data_size >= min_header_req + sizeof(BlockHeader) + _Alignof(max_align_t)) {
             size_t new_size = aligned_n;
             size_t leftover = current_data_size - new_size;
             
             BlockHeader *new_block = (BlockHeader *)((char *)block + header_size + new_size);
             new_block->size = leftover;
             new_block->is_free = 1;
             new_block->next = block->next;
             new_block->prev = block;
             
             if (block->next) {
                 block->next->prev = new_block;
             }
             block->next = new_block;
             block->size = new_size;
             
             // Merge new_block with next if next is free
             if (new_block->next && new_block->next->is_free) {
                 BlockHeader *next_free = new_block->next;
                 new_block->size += header_size + next_free->size;
                 new_block->next = next_free->next;
                 if (next_free->next) {
                     next_free->next->prev = new_block;
                 }
             }
        }
        return p;
    }

    // Need more space
    // Check if next block is free and large enough
    if (block->next && block->next->is_free) {
        size_t combined_size = block->size + header_size + block->next->size;
        if (combined_size >= aligned_n) {
            // Merge with next
            BlockHeader *next = block->next;
            block->size += header_size + next->size;
            block->next = next->next;
            if (next->next) {
                next->next->prev = block;
            }
            
            // Now check if we can split
            if (block->size >= aligned_n + header_size + sizeof(BlockHeader) + _Alignof(max_align_t)) {
                 size_t new_size = aligned_n;
                 size_t leftover = block->size - new_size;
                 BlockHeader *new_block = (BlockHeader *)((char *)block + header_size + new_size);
                 new_block->size = leftover;
                 new_block->is_free = 1;
                 new_block->next = block->next;
                 new_block->prev = block;
                 if (block->next) block->next->prev = new_block;
                 block->next = new_block;
                 block->size = new_size;
            }
            return p;
        }
    }

    // Cannot grow in place, must move
    void *new_ptr = arena_alloc(aligned_n);
    if (!new_ptr) return NULL;

    size_t copy_size = (n < block->size) ? n : block->size;
    // Copy data
    void *old_data = (char *)block + header_size;
    old_data = align_ptr(old_data, _Alignof(max_align_t));
    // We need the actual start of the allocated memory for copy
    // The alignment might have shifted the start relative to header + header_size?
    // No, align_ptr returns the aligned address.
    // We need to copy from old aligned address to new aligned address.
    // But we don't know the alignment offset from the header easily without storing it or recalculating.
    // Actually, align_ptr((char *)block + sizeof(BlockHeader), align) is deterministic.
    
    void *old_aligned = align_ptr((char *)block + header_size, _Alignof(max_align_t));
    void *new_aligned = align_ptr((char *)new_ptr - sizeof(BlockHeader) + header_size, _Alignof(max_align_t)); 
    // Wait, new_ptr is the user pointer. We need to find the header for new_ptr to calculate offset?
    // No, new_ptr IS the aligned user pointer.
    // We just need to copy from old_aligned to new_ptr.
    
    // How much to copy? min(n, old_data_size)
    // old_data_size is block->size.
    
    if (copy_size > 0) {
        // We need the exact byte count to copy.
        // The user expects 'n' bytes. The old block has 'block->size' bytes.
        // We copy min(n, block->size).
        // But wait, the data in the block is exactly 'block->size' bytes.
        // The user asked for 'n'.
        // We copy min(n, block->size) bytes from old_aligned to new_ptr.
        
        // However, we must be careful. The 'block->size' is the size of the data area.
        // So we copy min(n, block->size) bytes.
        
        // Wait, the alignment offset is constant.
        // old_aligned is the start of the data.
        // new_ptr is the start of the new data.
        
        // Let's just use memcpy.
        // We need to calculate the actual number of bytes to copy.
        // It's min(n, block->size).
        // But block->size includes the alignment padding? No, block->size is the data size.
        // The user allocated 'block->size' bytes.
        // So we copy min(n, block->size).
        
        // Actually, the prompt says "Contents up to the smaller of the old and new sizes are preserved".
        // Old size = block->size. New size = n.
        size_t bytes_to_copy = (n < block->size) ? n : block->size;
        
        // We need to copy from old_aligned to new_ptr.
        // But we don't have a direct way to get 'old_aligned' from 'p' without knowing the alignment offset.
        // But p IS old_aligned.
        // So memcpy(p, new_ptr, bytes_to_copy) is wrong. We copy FROM p TO new_ptr.
        memcpy(new_ptr, p, bytes_to_copy);
    }

    arena_free(p);
    return new_ptr;
}