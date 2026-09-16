#include <stddef.h>
#include <stdalign.h>
#include <stdint.h>
#include <string.h>

static void *g_buf = NULL;
static size_t g_size = 0;
static size_t g_offset = 0;

#define ALIGN(size) (((size) + (sizeof(max_align_t) - 1)) & ~(sizeof(max_align_t) - 1))

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
} block_header_t;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    g_offset = 0;
}

static inline block_header_t* get_header(void *p) {
    return (block_header_t*)((char*)p - sizeof(block_header_t));
}

static inline void* get_ptr(block_header_t *h) {
    return (void*)((char*)h + sizeof(block_header_t));
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_buf == NULL) return NULL;

    size_t aligned_n = ALIGN(n);
    size_t total_req = sizeof(block_header_t) + aligned_n;

    if (total_req > g_size) return NULL;

    block_header_t *current = (block_header_t*)((char*)g_buf + g_offset);

    // Check if we have enough space from current offset
    if (g_offset + total_req > g_size) return NULL;

    current->size = aligned_n;
    current->is_free = 0;
    current->next = NULL;

    g_offset += total_req;
    return get_ptr(current);
}

void arena_free(void *p) {
    if (p == NULL || g_buf == NULL) return;

    block_header_t *h = get_header(p);
    
    // Basic validation to ensure we are freeing within our arena
    if ((char*)h < (char*)g_buf || (char*)h >= (char*)g_buf + g_size) return;

    h->is_free = 1;

    // Merge with next block if it is free
    block_header_t *next = (block_header_t*)((char*)h + sizeof(block_header_t) + h->size);
    if ((char*)next < (char*)g_buf + g_size) {
        size_t next_offset = (char*)next - (char*)g_buf;
        // Check if next is within bounds and is a valid block (simplified check)
        // Since we don't have a linked list of all blocks, we rely on the fact that
        // any block after the current one in the linear buffer might be free.
        // However, without a global linked list, we need a way to find the next block.
        // In a simple linear allocator, the "next" block is simply the immediate neighbor.
        // But we only know a block is valid if it was allocated. 
        // To merge, we check if the immediate neighbor is free.
        // We need to know the size of the current block to find the next header.
        
        // Wait, the requirement says "merge with a free neighbour". 
        // In a linear bump allocator, blocks are contiguous. 
        // If block A is freed, and block B (immediately following) is free, we merge.
        // But how do we know B is free? We need to check if B was allocated as a block.
        // In this simple implementation, we assume if the memory at that location
        // contains a header with is_free=1, it's a valid free block.
        // However, we don't have a way to iterate all blocks. 
        // The standard approach for a simple arena with merge is to keep a free list 
        // or a linked list of headers. The prompt says "All bookkeeping must live inside that buffer".
        
        // Let's re-architect slightly to support merging properly.
        // We will maintain a linked list of headers within the buffer.
        // But the prompt asks for a specific set of functions.
        // Let's assume a simpler model: we keep a global linked list of free blocks? 
        // No, "All bookkeeping must live inside that buffer".
        
        // Revised Strategy:
        // The buffer contains headers. We maintain a linked list of headers (free or used) 
        // embedded in the headers themselves. 
        // We need a root pointer for the list. But we can't use global vars for the list 
        // if "all bookkeeping" is strictly inside the buffer? 
        // Actually, "bookkeeping" usually refers to metadata like size/flags. 
        // Pointers to the start of the list can be global.
        // Let's add a global head pointer for the list of all blocks.
    }
}

// Re-implementation with proper linked list for merging
static block_header_t *g_head = NULL;

void arena_init(void *buf, size_t size) {
    g_buf = buf;
    g_size = size;
    g_offset = 0;
    g_head = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0 || g_buf == NULL) return NULL;

    size_t aligned_n = ALIGN(n);
    size_t total_req = sizeof(block_header_t) + aligned_n;

    if (total_req > g_size) return NULL;

    // Find first free block that fits (first-fit strategy for reuse)
    block_header_t *prev = NULL;
    block_header_t *curr = g_head;

    while (curr != NULL) {
        if (curr->is_free && curr->size >= aligned_n) {
            // Check for exact fit or if we can split
            size_t remaining = curr->size - aligned_n;
            if (remaining > sizeof(block_header_t) + sizeof(max_align_t)) {
                // Split the block
                block_header_t *new_free = (block_header_t*)((char*)curr + sizeof(block_header_t) + aligned_n);
                new_free->size = remaining - sizeof(block_header_t);
                new_free->is_free = 1;
                new_free->next = curr->next;
                curr->size = aligned_n;
                curr->next = new_free;
            }
            curr->is_free = 0;
            return get_ptr(curr);
        }
        prev = curr;
        curr = curr->next;
    }

    // If no free block found, allocate new space at the end
    size_t current_used = 0;
    block_header_t *last = NULL;
    curr = g_head;
    while (curr != NULL) {
        current_used += sizeof(block_header_t) + curr->size;
        last = curr;
        curr = curr->next;
    }

    if (current_used + total_req > g_size) return NULL;

    curr = (block_header_t*)((char*)g_buf + current_used);
    curr->size = aligned_n;
    curr->is_free = 0;
    curr->next = NULL;

    if (last == NULL) {
        g_head = curr;
    } else {
        last->next = curr;
    }

    return get_ptr(curr);
}

void arena_free(void *p) {
    if (p == NULL || g_buf == NULL) return;

    block_header_t *h = get_header(p);
    
    // Validate pointer
    if ((char*)h < (char*)g_buf || (char*)h >= (char*)g_buf + g_size) return;

    h->is_free = 1;

    // Merge with next block if free
    block_header_t *next = h->next;
    if (next != NULL && next->is_free) {
        h->size += sizeof(block_header_t) + next->size;
        h->next = next->next;
    }

    // Merge with previous block if free
    block_header_t *prev = NULL;
    curr = g_head;
    while (curr != NULL && curr != h) {
        prev = curr;
        curr = curr->next;
    }

    if (prev != NULL && prev->is_free) {
        prev->size += sizeof(block_header_t) + h->size;
        prev->next = h->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    size_t aligned_n = ALIGN(n);
    block_header_t *h = get_header(p);
    size_t old_size = h->size;

    // Check if we can expand in place
    if (old_size >= aligned_n) {
        // Check if we can merge with next block to satisfy the request? 
        // Actually, if old_size >= aligned_n, we just return p.
        // But if we want to shrink, we might split. The prompt says:
        // "when the block immediately after p is free and large enough it must grow into that neighbour"
        // This implies if we need MORE space, we try to merge.
        // If we need LESS, we just return.
        // Let's handle the expansion case first.
        
        if (old_size >= aligned_n) {
            // If we are shrinking, we could split, but prompt doesn't explicitly require splitting on shrink.
            // It just says "Contents up to the smaller... preserved".
            // So if we don't need to split, just return.
            return p;
        }
    }

    // Need more space. Check if next block is free and large enough
    block_header_t *next = h->next;
    if (next != NULL && next->is_free) {
        size_t total_avail = old_size + sizeof(block_header_t) + next->size;
        if (total_avail >= aligned_n) {
            // Merge
            h->size = total_avail;
            // If we still have extra space, split it
            size_t remaining = h->size - aligned_n;
            if (remaining > sizeof(block_header_t) + sizeof(max_align_t)) {
                block_header_t *new_free = (block_header_t*)((char*)h + sizeof(block_header_t) + aligned_n);
                new_free->size = remaining - sizeof(block_header_t);
                new_free->is_free = 1;
                new_free->next = next->next;
                h->size = aligned_n;
                h->next = new_free;
            } else {
                // No split needed, just use all
                h->size = aligned_n;
                h->next = next->next;
            }
            h->is_free = 0;
            return p;
        }
    }

    // Cannot expand in place, allocate new
    void *new_ptr = arena_alloc(n);
    if (new_ptr == NULL) return NULL;

    // Copy data
    size_t copy_size = n < old_size ? n : old_size;
    memcpy(new_ptr, p, copy_size);

    arena_free(p);
    return new_ptr;
}