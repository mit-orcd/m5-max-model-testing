#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

struct block_header {
    size_t size;
    int is_free;
    int prev_free;
    int next_free;
};

#define HEADER_SIZE (sizeof(struct block_header) + _Alignof(max_align_t) - 1)
#define ALIGN_MASK (_Alignof(max_align_t) - 1)
#define ROUND_UP(n) (((n) + ALIGN_MASK) & ~ALIGN_MASK)

static void *arena_base;
static size_t arena_capacity;
static struct block_header *arena_first_free;

static struct block_header *get_header(void *p) {
    return (struct block_header *)((char *)p - HEADER_SIZE);
}

static struct block_header *get_next_block(struct block_header *h) {
    char *next = (char *)h + HEADER_SIZE + h->size;
    if ((char *)next >= (char *)arena_base + arena_capacity) {
        return NULL;
    }
    return (struct block_header *)next;
}

static struct block_header *get_prev_block(struct block_header *h) {
    if ((char *)h <= (char *)arena_base + HEADER_SIZE) {
        return NULL;
    }
    struct block_header *prev = get_next_block(NULL); // Start from beginning to find prev? No.
    // We need to scan forward to find the one pointing to h, or store prev pointers.
    // Since we don't store prev pointers in the header for simplicity of fixed buffer,
    // and to satisfy O(N) worst case for merging which is acceptable for arena allocators,
    // we will scan from the start.
    // However, standard bump allocators don't scan. With coalescing, we need to be careful.
    // Let's implement a simple linear scan for finding the previous block header during merge.
    
    struct block_header *curr = (struct block_header *)(arena_base);
    while (curr && curr != h) {
        struct block_header *next = get_next_block(curr);
        if (next == h) {
            return curr;
        }
        curr = next;
    }
    return NULL;
}

static void coalesce(struct block_header *h) {
    if (!h->is_free) return;

    // Merge with next
    struct block_header *next = get_next_block(h);
    if (next && next->is_free) {
        h->size += HEADER_SIZE + next->size;
        // Remove next from free list if it was there
        if (next->prev_free) arena_first_free = next; // Logic for free list removal is complex in O(1) without doubly linked list
        // Actually, let's maintain a simple singly linked list of free blocks? 
        // The prompt requires merging. Let's rebuild the free list or just do a full scan for O(N) safety.
        // To keep it robust and simple: Scan all blocks to rebuild free list or just mark coalesced.
        // Let's do a full scan to update free list pointers and merge correctly.
    }

    // Merge with prev
    struct block_header *prev = get_prev_block(h);
    if (prev && prev->is_free) {
        // Merge h into prev
        prev->size += HEADER_SIZE + h->size;
        // Remove h from free list
        if (h->prev_free) {
            struct block_header *tmp = arena_first_free;
            while (tmp != h && tmp != NULL) {
                tmp = (struct block_header *)((char *)tmp + HEADER_SIZE + tmp->size); // This is wrong for free list traversal
                // We need a proper free list structure.
                // Let's change strategy: Use a simple linked list for free blocks stored inside the buffer.
                // We will store the 'next_free' pointer in the header itself, but we need 'prev_free' for O(1) removal?
                // Or just O(N) removal.
            }
        }
        // Since we are modifying h, and h is free, we effectively remove h.
        // Let's re-implement the free list logic to be robust.
        // We will maintain a singly linked list of free blocks using the 'next_free' field (reused from struct).
        // But we need to update pointers.
    }
}

// Redefining implementation to be strictly correct and self-contained
// We will use a doubly linked list for free blocks to allow O(1) removal and merging.
// We will store 'next_free' and 'prev_free' in the header.

struct block_header_full {
    size_t size;
    int is_free;
    struct block_header_full *next_free;
    struct block_header_full *prev_free;
};

static struct block_header_full *arena_free_list;

void arena_init(void *buf, size_t size) {
    arena_base = buf;
    arena_capacity = size;
    arena_free_list = NULL;

    if (size <= HEADER_SIZE) return;

    // Create one big free block
    struct block_header_full *h = (struct block_header_full *)arena_base;
    h->size = size - HEADER_SIZE;
    h->is_free = 1;
    h->next_free = NULL;
    h->prev_free = NULL;
    arena_free_list = h;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t aligned_n = ROUND_UP(n);
    struct block_header_full *h = arena_free_list;
    
    while (h) {
        if (h->is_free && h->size >= aligned_n) {
            // Found a block
            h->is_free = 0;
            
            // Remove from free list
            if (h->prev_free) h->prev_free->next_free = h->next_free;
            else arena_free_list = h->next_free;
            
            if (h->next_free) h->next_free->prev_free = h->prev_free;
            h->next_free = NULL;
            h->prev_free = NULL;

            // Split if enough space left
            size_t remaining = h->size - aligned_n;
            if (remaining >= HEADER_SIZE + _Alignof(max_align_t)) {
                struct block_header_full *new_h = (struct block_header_full *)((char *)h + HEADER_SIZE + aligned_n);
                new_h->size = remaining - HEADER_SIZE;
                new_h->is_free = 1;
                new_h->next_free = NULL;
                new_h->prev_free = NULL;
                
                // Insert new_h at head of free list
                new_h->next_free = arena_free_list;
                if (arena_free_list) arena_free_list->prev_free = new_h;
                arena_free_list = new_h;
            }
            
            return (char *)h + HEADER_SIZE;
        }
        h = h->next_free;
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    if (p < arena_base || (char *)p >= (char *)arena_base + arena_capacity) return;

    struct block_header_full *h = get_header(p);
    h->is_free = 1;

    // Add to free list (head)
    h->next_free = arena_free_list;
    h->prev_free = NULL;
    if (arena_free_list) arena_free_list->prev_free = h;
    arena_free_list = h;

    // Coalesce with next
    struct block_header_full *next = (struct block_header_full *)((char *)h + HEADER_SIZE + h->size);
    if ((char *)next < (char *)arena_base + arena_capacity && next->is_free) {
        // Remove next from free list
        if (next->prev_free) next->prev_free->next_free = next->next_free;
        else if (next == arena_free_list) arena_free_list = next->next_free; // Should be handled by prev logic usually, but safe check
        
        if (next->next_free) next->next_free->prev_free = next->prev_free;
        else {
            // next was tail, update the new tail's prev if any
            // Actually, if next was tail, next->next_free is NULL.
            // We need to find the new tail? No, we just remove it.
            // If next was the last one, we need to update the one before it.
            // But we already did: if (next->prev_free) ...
            // If next was head and removed, arena_free_list updated.
            // Wait, if next was head (arena_free_list), and we set arena_free_list = next->next_free.
            // But we just added h to head. So next cannot be head unless h was the only one?
            // If h was head, next is second.
            // Logic: next is free. Remove next.
            if (next->prev_free == NULL && next == arena_free_list) {
                 // This case happens if h was the only free block? No, h is not in list yet?
                 // We added h to list. So next cannot be head unless h and next are same? Impossible.
                 // Wait, if h was head, we set h->next_free = old_head.
                 // So next is old_head+1? No.
                 // Let's re-evaluate: h is now head. next is the physical next block.
                 // If next is free, it must be in the list (not necessarily head).
                 // Remove next.
                 if (next->prev_free) next->prev_free->next_free = next->next_free;
                 else arena_free_list = next->next_free; // next was head? Impossible if h is head and h != next.
                 
                 if (next->next_free) next->next_free->prev_free = next->prev_free;
            } else {
                // Standard removal
                if (next->prev_free) next->prev_free->next_free = next->next_free;
                else arena_free_list = next->next_free;
                if (next->next_free) next->next_free->prev_free = next->prev_free;
            }

            h->size += HEADER_SIZE + next->size;
        }
    }

    // Coalesce with prev
    struct block_header_full *prev = (struct block_header_full *)((char *)h - HEADER_SIZE);
    if ((char *)prev >= (char *)arena_base + HEADER_SIZE && prev->is_free) {
        // Remove h from free list (it's currently head)
        // h is head because we just added it.
        // So next_free of h is the one after it (if any).
        // We are merging h into prev.
        
        // Remove h
        if (h->next_free) h->next_free->prev_free = NULL; // h was head, so next becomes new head
        else arena_free_list = NULL; // h was the only one
        
        h->next_free = NULL;
        h->prev_free = NULL;
        
        // Remove prev from list
        if (prev->prev_free) prev->prev_free->next_free = prev->next_free;
        else arena_free_list = prev->next_free;
        
        if (prev->next_free) prev->next_free->prev_free = prev->prev_free;
        
        prev->size += HEADER_SIZE + h->size;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    struct block_header_full *h = get_header(p);
    size_t old_size = h->size;
    size_t aligned_n = ROUND_UP(n);

    // Check if we can grow in place
    if (aligned_n <= old_size) {
        return p; // Already fits
    }

    // Check next block
    struct block_header_full *next = (struct block_header_full *)((char *)h + HEADER_SIZE + old_size);
    if ((char *)next < (char *)arena_base + arena_capacity && next->is_free) {
        size_t next_size = next->size;
        size_t total = old_size + HEADER_SIZE + next_size;
        
        if (total >= aligned_n) {
            // Merge next into current
            // Remove next from free list
            if (next->prev_free) next->prev_free->next_free = next->next_free;
            else arena_free_list = next->next_free;
            
            if (next->next_free) next->next_free->prev_free = next->prev_free;
            
            h->size += HEADER_SIZE + next_size;
            return p;
        }
    }

    // Need to move
    void *new_p = arena_alloc(aligned_n);
    if (new_p == NULL) return NULL;

    // Copy min(old, new)
    size_t copy_size = (old_size < aligned_n) ? old_size : aligned_n;
    memcpy(new_p, p, copy_size);

    arena_free(p);
    return new_p;
}

// Helper to get header for reuse in free/realloc
// Note: The get_header function used in arena_free and arena_realloc above needs to be defined
// inside the code block properly.
// I will define it as a static inline or just duplicate logic if needed, but let's add it.
static struct block_header_full *get_header(void *p) {
    return (struct block_header_full *)((char *)p - HEADER_SIZE);
}