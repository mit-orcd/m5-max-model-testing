#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* 
 * We use a simple header structure for each block.
 * The header contains the size of the block (including header) and a flag indicating if it's free.
 * All bookkeeping is inside the buffer provided to arena_init.
 * 
 * Layout:
 * The buffer starts with a header for the entire arena? 
 * Actually, we manage the buffer as a series of blocks. Each block has a header.
 * We need to store the total size of the buffer and the current free list or just traverse blocks.
 * 
 * Approach: 
 * - The entire buffer is treated as one big block initially.
 * - Each allocated or free block has a header: { size_t size; int free; }
 * - But we also need to know the end of the arena to prevent overflow and to handle the last block.
 * - We store metadata at the very beginning of the buffer:
 *   struct arena_header {
 *       size_t total_size;
 *       size_t used; // not strictly needed if we traverse
 *   };
 *   Actually, simpler: just put a header at the start of the buffer that marks the whole thing as one free block?
 *   But we need to know the total size to know when we've reached the end during traversal.
 *   
 * Let's do this:
 * The buffer is aligned to max_align_t.
 * The first thing in the buffer is an `arena_meta`:
 *   size_t total_size;
 * Then immediately after that, the first block header starts.
 * 
 * However, to simplify alignment and management, let's say:
 * The buffer is divided into blocks. Each block starts with a `block_header`:
 *   size_t size; // total size of this block including this header
 *   int free;    // 1 if free, 0 if allocated
 * 
 * To know the end of the arena, we store the total buffer size in a fixed location at the start of the buffer.
 * 
 * Let `buf` be the start of the buffer.
 * We reserve space for `size_t total_size` at the beginning, properly aligned.
 * Since `buf` is already aligned to max_align_t (as per standard malloc-like semantics, but arena_init receives a buffer, we assume it's sufficiently aligned or we adjust).
 * 
 * Actually, the problem says "arena_init hands the allocator a buffer". We should assume the buffer is aligned suitably or we handle alignment.
 * 
 * Plan:
 * 1. In arena_init, we store the total size at the beginning of the buffer.
 *    We need to make sure the first block header is aligned to max_align_t.
 *    Let's use a struct for the arena metadata:
 *    
 *    typedef struct {
 *        size_t total_size;
 *    } arena_meta;
 *    
 *    But max_align_t alignment is needed for the blocks.
 *    
 *    Let's define the block header:
 *    typedef struct {
 *        size_t size; // size of the entire block (header + data)
 *        size_t free; // 1 if free, 0 if used
 *    } block_header;
 *    
 *    The size of block_header must be a multiple of _Alignof(max_align_t) to ensure the next block is aligned.
 *    
 *    In arena_init:
 *    - Check if the buffer size is large enough to hold at least the meta and one block.
 *    - Store total_size at the beginning.
 *    - The first block header starts right after the meta, aligned to max_align_t.
 *    - The first block is free and spans the rest of the buffer.
 * 
 * 2. arena_alloc(n):
 *    - If n == 0, return NULL.
 *    - Align n to max_align_t? The returned pointer must be suitably aligned. Since block headers are aligned and data follows, if header size is multiple of max_align_t, data is aligned.
 *    - Traverse blocks from the first block.
 *    - For each block, if it's free and its size (excluding header? no, total block size) >= header_size + aligned_n, we can use it.
 *    - If the block is larger than needed, split it.
 *    - Mark the block as used.
 *    - Return pointer to data (after header).
 * 
 * 3. arena_free(p):
 *    - If p is NULL, return.
 *    - Get the block header from p (p - sizeof(block_header)).
 *    - Mark the block as free.
 *    - Merge with the next block if it exists and is free.
 *    - Merge with the previous block if it exists and is free.
 * 
 * 4. arena_realloc(p, n):
 *    - If p is NULL, return arena_alloc(n).
 *    - If n == 0, arena_free(p); return NULL.
 *    - Get the current block size.
 *    - If the current block is large enough for n, return p.
 *    - If the next block is free and combined size (current + next) >= needed, grow into next block, return p.
 *    - Else, allocate new block, copy min(old_size_data, new_size_data), free old, return new.
 * 
 * Implementation details:
 * - We need a way to find the last block or know when we've reached the end of the buffer.
 * - Store total_size at the beginning. The first block header starts at offset `meta_size` (aligned).
 * - During traversal, we know we're at the end when the block header pointer + block_size > buffer_end.
 * 
 * Let's define:
 * #define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))
 * 
 * The block header itself must be aligned. Let's ensure sizeof(block_header) is a multiple of _Alignof(max_align_t).
 * 
 * We'll use a static global variable to store the buffer start and total size? No, "all bookkeeping must live inside that buffer". So we must retrieve state from the buffer itself.
 * 
 * But arena_alloc and arena_free don't receive the buffer pointer. How do they find the buffer?
 * 
 * This is a problem. The functions arena_alloc, arena_free, arena_realloc don't take the buffer pointer. They must know the buffer location.
 * 
 * Re-reading: "arena_init hands the allocator a buffer and resets all state. All bookkeeping must live inside that buffer".
 * 
 * This implies that there's some global state that points to the buffer? But "do not call malloc" and "all bookkeeping must live inside that buffer".
 * 
 * If we have multiple arenas, this design fails. But the problem doesn't mention multiple arenas. It seems like a single global arena.
 * 
 * However, "all bookkeeping must live inside that buffer" suggests that even the pointer to the buffer's start should be stored inside the buffer? That's circular.
 * 
 * Let's re-read carefully: "arena_init hands the allocator a buffer and resets all state."
 * 
 * Typically, in such problems, there's a global variable that stores the buffer pointer. But the constraint "All bookkeeping must live inside that buffer" might refer to the allocation metadata (free lists, block sizes), not the pointer to the buffer itself.
 * 
 * Or, it could mean that we use a single global variable `arena_buf` and `arena_size` that are set by arena_init, and all the block headers are inside the buffer. The "bookkeeping" refers to the block headers and free lists, not the global pointer.
 * 
 * Given the function signatures, we MUST have a global variable to remember the buffer. Otherwise, arena_alloc has no way to know where the arena is.
 * 
 * So, I'll use global variables `arena_start` and `arena_total_size` set by arena_init. The "bookkeeping inside the buffer" refers to the block headers, not these globals.
 */

typedef struct {
    size_t size; // Total size of the block including this header
    int free;    // 1 if free, 0 if allocated
} block_header;

static void *arena_start = NULL;
static size_t arena_total_size = 0;

#define MAX_ALIGN _Alignof(max_align_t)

static size_t align_up(size_t x, size_t a) {
    return (x + a - 1) & ~(a - 1);
}

static size_t header_size_aligned(void) {
    return align_up(sizeof(block_header), MAX_ALIGN);
}

static void *get_block_data(block_header *hdr) {
    return (void*)((char*)hdr + header_size_aligned());
}

static block_header *get_block_header(void *p) {
    return (block_header*)((char*)p - header_size_aligned());
}

static block_header *get_next_block(block_header *hdr) {
    char *next = (char*)hdr + hdr->size;
    if ((void*)next >= (void*)((char*)arena_start + arena_total_size)) {
        return NULL;
    }
    return (block_header*)next;
}

static block_header *get_first_block(void) {
    // The first block starts after the initial metadata area.
    // We'll just say the buffer starts with the first block header.
    // But we need to know the total size. Let's reserve the first few bytes for total_size.
    // Actually, let's store total_size at the beginning of the buffer, aligned.
    
    // Let's define the layout:
    // [size_t total_size][padding for alignment][first block header][data]...
    
    // But arena_start points to the very beginning of the buffer.
    // Let's store total_size at arena_start.
    // The first block header starts at arena_start + sizeof(size_t), aligned to MAX_ALIGN.
    
    size_t meta_size = align_up(sizeof(size_t), MAX_ALIGN);
    return (block_header*)((char*)arena_start + meta_size);
}

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size == 0) {
        arena_start = NULL;
        arena_total_size = 0;
        return;
    }
    
    // Ensure proper alignment of buf? The caller should provide aligned buf, but let's be safe.
    // Actually, we just use it as is.
    
    size_t meta_size = align_up(sizeof(size_t), MAX_ALIGN);
    if (size < meta_size + header_size_aligned()) {
        arena_start = NULL;
        arena_total_size = 0;
        return;
    }
    
    arena_start = buf;
    arena_total_size = size;
    
    // Store total size at the beginning
    *(size_t*)buf = size;
    
    // Initialize the first block as free
    block_header *first = get_first_block();
    first->size = size - meta_size;
    first->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (arena_start == NULL) return NULL;
    
    size_t aligned_n = align_up(n, MAX_ALIGN);
    size_t needed = header_size_aligned() + aligned_n;
    
    block_header *curr = get_first_block();
    char *end = (char*)arena_start + arena_total_size;
    
    while ((void*)curr < (void*)end) {
        if (curr->free && curr->size >= needed) {
            // Found a suitable block
            if (curr->size > needed + header_size_aligned()) {
                // Split the block
                block_header *new_block = (block_header*)((char*)curr + needed);
                new_block->size = curr->size - needed;
                new_block->free = 1;
                curr->size = needed;
            }
            curr->free = 0;
            return get_block_data(curr);
        }
        curr = get_next_block(curr);
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    if (arena_start == NULL) return;
    
    block_header *hdr = get_block_header(p);
    hdr->free = 1;
    
    // Merge with next block if free
    block_header *next = get_next_block(hdr);
    if (next != NULL && next->free) {
        hdr->size += next->size;
    }
    
    // Merge with previous block if free
    // To find previous block, we need to traverse from the start.
    // This is O(n). For simplicity, we'll do it.
    block_header *curr = get_first_block();
    block_header *prev = NULL;
    while (curr != NULL && curr != hdr) {
        prev = curr;
        curr = get_next_block(curr);
    }
    
    if (prev != NULL && prev->free) {
        prev->size += hdr->size;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    block_header *hdr = get_block_header(p);
    size_t current_data_size = hdr->size - header_size_aligned();
    size_t aligned_n = align_up(n, MAX_ALIGN);
    size_t needed = header_size_aligned() + aligned_n;
    
    // If current block is large enough
    if (hdr->size >= needed) {
        return p;
    }
    
    // Check if next block is free and can be merged
    block_header *next = get_next_block(hdr);
    if (next != NULL && next->free && hdr->size + next->size >= needed) {
        // Grow into next block
        size_t old_data_size = current_data_size;
        hdr->size += next->size;
        hdr->free = 0; // It was already 0 if p is valid allocated
        
        // If the combined block is much larger than needed, we could split, but realloc doesn't require shrinking.
        // Just return p.
        return p;
    }
    
    // Allocate new block
    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    
    // Copy data
    size_t copy_size = current_data_size < n ? current_data_size : n;
    memcpy(new_p, p, copy_size);
    
    // Free old block
    arena_free(p);
    
    return new_p;
}