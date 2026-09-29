```c
#include <stddef.h>
#include <stdalign.h>

#define ARENA_MAGIC 0xA110CA7E

typedef struct BlockHeader {
    size_t size;
    int is_free;
    unsigned int magic;
} BlockHeader;

static void *g_buf = NULL;
static size_t g_size = 0;
static BlockHeader *g_head = NULL;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    
    if (size == 0 || buf == NULL) {
        g_head = NULL;
        return;
    }

    // Align the start of the usable space
    size_t align_offset = (-(size_t)buf) & (alignof(max_align_t) - 1);
    if (align_offset == 0) align_offset = 0;
    
    // Ensure we have space for at least one header
    if (align_offset >= size - sizeof(BlockHeader)) {
        g_head = NULL;
        return;
    }

    char *start = (char *)buf + align_offset;
    g_head = (BlockHeader *)start;
    g_head->size = size - align_offset - sizeof(BlockHeader);
    g_head->is_free = 1;
    g_head->magic = ARENA_MAGIC;
}

static BlockHeader *get_header(void *p) {
    if (p == NULL) return NULL;
    BlockHeader *h = ((BlockHeader *)p) - 1;
    if (h < g_head || h > (BlockHeader *)((char *)g_buf + g_size)) return NULL;
    if (h->magic != ARENA_MAGIC) return NULL;
    return h;
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_head == NULL) return NULL;

    // Align request size
    size_t align = alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    
    // Minimum block size to ensure header fits and alignment
    if (aligned_n < sizeof(BlockHeader)) {
        aligned_n = sizeof(BlockHeader);
    }

    // First fit search
    BlockHeader *curr = g_head;
    while (curr != NULL) {
        if (curr->is_free && curr->size >= aligned_n) {
            // Found a suitable block
            size_t remaining = curr->size - aligned_n;
            
            // If remaining space is enough to hold a header and some data, split it
            if (remaining > sizeof(BlockHeader) + sizeof(max_align_t)) {
                // Split the block
                BlockHeader *new_block = (BlockHeader *)((char *)curr + aligned_n + sizeof(BlockHeader) - sizeof(BlockHeader));
                // Actually, the data starts at curr + 1 (after header)
                // The new block header goes after the allocated data
                char *data_start = (char *)curr + sizeof(BlockHeader);
                char *new_header_ptr = data_start + aligned_n;
                
                new_block = (BlockHeader *)new_header_ptr;
                new_block->size = remaining - sizeof(BlockHeader);
                new_block->is_free = 1;
                new_block->magic = ARENA_MAGIC;
                
                curr->size = aligned_n;
            }
            
            curr->is_free = 0;
            return (void *)((char *)curr + sizeof(BlockHeader));
        }
        
        if (curr->is_free) {
            curr = (BlockHeader *)((char *)curr + curr->size + sizeof(BlockHeader));
        } else {
            curr = (BlockHeader *)((char *)curr + curr->size + sizeof(BlockHeader));
        }
        
        // Check if we've gone past the buffer
        if ((char *)curr > (char *)g_buf + g_size) {
            break;
        }
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    BlockHeader *h = get_header(p);
    if (h == NULL) return;

    h->is_free = 1;

    // Merge with next block if free
    char *end_of_curr = (char *)h + sizeof(BlockHeader) + h->size;
    if (end_of_curr < (char *)g_buf + g_size) {
        BlockHeader *next = (BlockHeader *)end_of_curr;
        if (next->magic == ARENA_MAGIC && next->is_free) {
            h->size += sizeof(BlockHeader) + next->size;
        }
    }

    // Merge with previous block if free
    if (h != g_head) {
        // Find previous block
        char *prev_start = (char *)g_buf;
        // We need to find the block before h. Since we don't have back pointers,
        // we scan from head. This is O(N) but necessary for a simple fixed buffer.
        BlockHeader *prev = g_head;
        while (prev != NULL) {
            char *prev_end = (char *)prev + sizeof(BlockHeader) + prev->size;
            if (prev_end == (char *)h) {
                break;
            }
            if (prev->is_free) {
                prev = (BlockHeader *)prev_end;
            } else {
                prev = (BlockHeader *)prev_end;
            }
            if (prev >= (BlockHeader *)((char *)g_buf + g_size)) {
                prev = NULL;
                break;
            }
        }
        
        if (prev != NULL && prev->is_free) {
            prev->size += sizeof(BlockHeader) + h->size;
            // Update head if necessary (though h was after head usually)
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

    BlockHeader *h = get_header(p);
    if (h == NULL) return NULL;

    size_t old_size = h->size;
    size_t align = alignof(max_align_t);
    size_t aligned_n = (n + align - 1) & ~(align - 1);
    if (aligned_n < sizeof(BlockHeader)) {
        aligned_n = sizeof(BlockHeader);
    }

    // If shrinking or same size, just keep it (no need to split unless we want to)
    // Actually, if we shrink significantly, we might leave a large free block.
    // But for simplicity, if new size <= old size, we just keep the block.
    // To be efficient, we should split if the remainder is large enough.
    
    if (aligned_n <= old_size) {
        size_t remaining = old_size - aligned_n;
        if (remaining > sizeof(BlockHeader) + sizeof(max_align_t)) {
            // Split
            char *data_start = (char *)h + sizeof(BlockHeader);
            char *new_header_ptr = data_start + aligned_n;
            BlockHeader *new_block = (BlockHeader *)new_header_ptr;
            new_block->size = remaining - sizeof(BlockHeader);
            new_block->is_free = 1;
            new_block->magic = ARENA_MAGIC;
            h->size = aligned_n;
            
            // Merge new block with its next neighbor if free
            char *new_end = (char *)new_block + sizeof(BlockHeader) + new_block->size;
            if (new_end < (char *)g_buf + g_size) {
                BlockHeader *next = (BlockHeader *)new_end;
                if (next->magic == ARENA_MAGIC && next->is_free) {
                    new_block->size += sizeof(BlockHeader) + next->size;
                    // Note: next block is effectively merged, so we don't touch its header directly
                    // but we updated new_block's size. The next block's header