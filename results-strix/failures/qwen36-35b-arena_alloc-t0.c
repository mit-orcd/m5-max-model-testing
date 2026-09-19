```c
#include <stddef.h>
#include <stdint.h>

#define ARENA_MAGIC 0xA110CA7E

typedef struct BlockHeader {
    size_t size;
    int is_free;
    uint32_t magic;
} BlockHeader;

static void *g_buf = NULL;
static size_t g_size = 0;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    
    // Initialize the first block
    BlockHeader *first = (BlockHeader *)buf;
    // Reserve space for header, ensure alignment for payload
    size_t header_size = (sizeof(BlockHeader) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    first->size = size - header_size;
    first->is_free = 1;
    first->magic = ARENA_MAGIC;
}

static BlockHeader *get_header(void *p) {
    return (BlockHeader *)((char *)p - sizeof(BlockHeader));
}

static void *get_payload(BlockHeader *hdr) {
    size_t header_size = (sizeof(BlockHeader) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    return (char *)hdr + header_size;
}

static BlockHeader *get_next_block(BlockHeader *hdr) {
    size_t header_size = (sizeof(BlockHeader) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    return (BlockHeader *)((char *)hdr + sizeof(BlockHeader) + hdr->size);
}

static int is_valid_block(BlockHeader *hdr) {
    return hdr->magic == ARENA_MAGIC;
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_buf == NULL) {
        return NULL;
    }

    size_t header_size = (sizeof(BlockHeader) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    size_t total_needed = header_size + n;
    
    // Align the requested size up to alignment boundary
    size_t aligned_n = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    total_needed = header_size + aligned_n;

    BlockHeader *current = (BlockHeader *)g_buf;
    
    while (is_valid_block(current)) {
        if (current->is_free && current->size >= aligned_n) {
            // Found a suitable block
            size_t remaining = current->size - aligned_n;
            
            // If the remaining space is large enough to hold a new block header + minimal payload, split it
            if (remaining > header_size + _Alignof(max_align_t)) {
                // Split the block
                BlockHeader *new_block = (BlockHeader *)((char *)current + header_size + aligned_n);
                new_block->size = remaining - header_size;
                new_block->is_free = 1;
                new_block->magic = ARENA_MAGIC;
                
                current->size = aligned_n;
            }
            
            current->is_free = 0;
            return get_payload(current);
        }
        
        // Move to next block
        current = get_next_block(current);
        
        // Check if we've reached the end of the buffer
        if ((char *)current >= (char *)g_buf + g_size) {
            break;
        }
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }
    
    BlockHeader *hdr = get_header(p);
    if (!is_valid_block(hdr)) {
        return;
    }
    
    hdr->is_free = 1;
    
    // Merge with next block if it is free
    BlockHeader *next = get_next_block(hdr);
    if (is_valid_block(next) && next->is_free) {
        hdr->size += sizeof(BlockHeader) + next->size;
    }
    
    // Merge with previous block if it is free
    // We need to find the previous block. Since we don't have a linked list,
    // we must scan from the beginning. This is O(N) but necessary for a simple arena.
    BlockHeader *prev = (BlockHeader *)g_buf;
    while (is_valid_block(prev)) {
        BlockHeader *next_prev = get_next_block(prev);
        if (next_prev == hdr) {
            if (prev->is_free) {
                prev->size += sizeof(BlockHeader) + hdr->size;
                // Note: We don't need to update next's header because we merged into prev
                // and the next pointer of the merged block is now effectively invalid/ignored
                // until we traverse again. The size of prev now covers the space of hdr and next.
            }
            break;
        }
        prev = next_prev;
        if ((char *)prev >= (char *)g_buf + g_size) {
            break;
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
    
    BlockHeader *hdr = get_header(p);
    if (!is_valid_block(hdr)) {
        return NULL;
    }
    
    size_t current_payload_size = hdr->size;
    size_t header_size = (sizeof(BlockHeader) + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    size_t aligned_n = (n + _Alignof(max_align_t) - 1) & ~(_Alignof(max_align_t) - 1);
    
    // If the new size is smaller or equal, and we don't need to shrink, just return p
    // However, standard realloc doesn't shrink unless necessary. 
    // If the block is large enough, we can keep it.
    if (current_payload_size >= aligned_n) {
        // Check if we can merge with next block if we were to shrink? 
        // No, realloc usually keeps the block if it fits.
        // But if we are growing, we might merge.
        // If we are shrinking, we just return the same pointer.
        // The prompt says: "when the block immediately after p is free and large enough it must grow into that neighbour"
        // This implies we only merge on growth.
        return p;
    }
    
    // We need to grow. Check if the next block is free and large enough to accommodate the growth
    BlockHeader *next = get_next_block(hdr);
    if (is_valid_block(next) && next->is_free) {
        size_t next_payload_size = next->size;
        size_t space_available = current_payload_size + sizeof(BlockHeader) + next_payload_size;
        
        if (space_available >= aligned_n) {
            // Grow into the next block
            size_t remaining = space_available - aligned_n;
            
            // If remaining space is large enough to hold a new block header + minimal payload, split it
            if (remaining > header_size + _Alignof(max_align_t)) {
                BlockHeader *new_block = (BlockHeader *)((char *)hdr + header_size + aligned_n);
                new_block->size = remaining - header_size;
                new_block->is_free = 1;
                new_block->magic = ARENA_MAGIC;
                
                hdr->size = aligned_n;
            } else {
                // Take all the space
                hdr->size = space_available;
            }
            
            // Copy data if the new size is larger than the old payload size
            // The prompt says: "Contents up to the smaller of the old and new sizes are preserved."
            // Since we are growing, old size < new size, so we preserve old size.