#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>

#define AHEAD_SIZE 32

typedef struct {
    int is_free;
    size_t size;
} Header;

typedef HeapArena {
    Header header;
    alignas(max_align_t) char data[1];
} Arena;

static Arena *arena;
static size_t arena_size;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(Header)) {
        return;
    }
    size_t usable = size - sizeof(Header);
    arena = (Arena*)buf;
    arena_size = size;

    arena->header.is_free = 1;
    arena->header.size = usable;
}

static inline void* block_ptr(Arena *h) {
    return (char*)h + sizeof(Header);
}

static inline Header* block_header(void *p) {
    return (Header*)((char*)p - sizeof(Header));
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    // Align n to max_align_t
    uintptr_t addr = (uintptr_t)arena->data;
    size_t offset = (addr + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);
    size_t aligned_n = offset - (uintptr_t)arena->data;
    size_t total_req = aligned_n + sizeof(Header);
    
    Header *h = arena;
    
    while (h) {
        if (h->is_free && h->size >= total_req) {
            h->is_free = 0;
            size_t remaining = h->size - total_req;
            
            if (remaining >= sizeof(Header)) {
                Header *next = (Header*)((char*)h + sizeof(Header) + aligned_n);
                next->is_free = 1;
                next->size = remaining - sizeof(Header);
            }
            
            return block_ptr(h);
        }
        
        if (h->size == 0) break; // End of arena
        
        h = (Header*)((char*)h + sizeof(Header) + h->size);
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    Header *h = block_header(p);
    Header *next = (Header*)((char*)h + sizeof(Header) + h->size);
    
    // Merge with next if free and valid
    if (next->is_free && (char*)next < (char*)arena + arena_size) {
        h->size += sizeof(Header) + next->size;
        next->is_free = 1; // Mark as merged (conceptually, we just expand h)
        // Logic: next is now part of h. We just need to ensure we don't traverse it as a separate block.
        // Actually, to simplify traversal, we just update h's size and leave next's data as garbage until overwritten or merged again.
        // But we need to handle the merge correctly. 
        // Let's fix the logic: merge next into h.
        h->size = h->size + sizeof(Header) + next->size;
        // We don't need to change next->is_free explicitly if we stop traversal at h, 
        // but we must ensure the next pointer calculation in loop skips the merged block.
        // Since h's size now covers next, the loop naturally skips it.
    }
    
    h->is_free = 1;
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);

    Header *h = block_header(p);
    size_t old_size = h->size;
    
    // Align n
    uintptr_t addr = (uintptr_t)block_ptr(h);
    size_t offset = (addr + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1);
    size_t aligned_old = offset - (uintptr_t)block_ptr(h); // 0 if already aligned
    size_t aligned_n = offset - (uintptr_t)block_ptr(h);
    size_t total_req = aligned_n + sizeof(Header);
    
    if (total_req <= old_size) {
        // Shrink or fit in place
        // No data movement needed if we just update size?
        // Wait, we must preserve data.
        // If we shrink, we just update header size.
        h->size = total_req;
        return p;
    }
    
    // Try to grow into next block
    Header *next = (Header*)((char*)h + sizeof(Header) + h->size);
    
    if (next->is_free && (char*)next < (char*)arena + arena_size) {
        size_t next_size = next->size;
        size_t new_total = total_req;
        
        if (new_total <= (size_t)sizeof(Header) + next_size) {
            // Merge next into h
            h->size = new_total;
            // Next is now part of h, so we effectively remove it from the list by expanding h.
            return p;
        }
    }
    
    // Fallback: Alloc new, copy, free old
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, (old_size < n) ? old_size : n);
        // Wait, old_size includes header? No, h->size is payload size.
        // We need to copy payload.
        memcpy(block_ptr(new_p), block_ptr(p), (old_size < n) ? old_size : n);
        arena_free(p);
    }
    return new_p;
}