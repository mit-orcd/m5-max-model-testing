#include <stddef.h>
#include <stdint.h>

// Arena buffer must be aligned to largest type and a power of two in size
#define ARENA_BUF_ALIGN 16
#define ARENA_BUF_SIZE  (64 * 1024)

// Header for each allocated block
typedef struct arena_block {
    struct arena_block *next;
    size_t             size;
} arena_block;

// The arena state
typedef struct {
    void              *buf;     // Base of arena buffer
    void              *alloc;   // Next allocation pointer
    arena_block       *free;    // List of free blocks
    size_t             free_sz;  // Total size of free blocks
} arena_state;

// Initialize the arena
void arena_init(void *buf, size_t size) {
    arena_state *s = (arena_state *)buf;
    s->buf = buf;
    s->alloc = (char*)buf + ARENA_BUF_ALIGN;
    s->free = NULL;
    s->free_sz = 0;
    // Initialize the free list with a single large block
    s->free = (arena_block*)s->alloc;
    s->free->next = NULL;
    s->free->size = size - ARENA_BUF_ALIGN;
    s->alloc = (char*)s->alloc + s->free->size;
}

// Find space for a new block of size 'n'
static void *arena_alloc_space(arena_state *s, size_t n) {
    size_t best_sz = 0;
    arena_block *best_block = NULL;
    // Try to find a free block large enough
    for (arena_block *block = s->free; block != NULL; block = block->next) {
        if (block->size >= n) {
            if (block->size > best_sz) {
                best_sz = block->size;
                best_block = block;
            }
        }
    }
    // If found, remove from free list and return space
    if (best_block) {
        s->free_sz -= best_block->size;
        if (s->free == best_block) {
            s->free = best_block->next;
        } else {
            arena_block *prev = best_block->next;
            prev->next = best_block->next;
        }
        return (char*)best_block + ARENA_BUF_ALIGN;
    }
    // Otherwise, try to split the largest free block
    if (s->free_sz > best_sz) {
        size_t split_sz = s->free->size / 2;
        if (split_sz >= n) {
            arena_block *new_block = (arena_block*)s->alloc;
            new_block->next = s->free;
            new_block->size = split_sz;
            s->alloc = (char*)s->alloc + split_sz;
            s->free_sz += split_sz;
            s->free = new_block;
            return (char*)new_block + ARENA_BUF_ALIGN;
        }
    }
    // If all else fails, return NULL
    return NULL;
}

// Merge free blocks on either side of 'p' if possible
static void arena_merge_free(arena_state *s, void *p) {
    arena_block *block = (arena_block*)p - 1;
    if (block->next && block->next->size + block->size >= block->next->size) {
        block->size += block->next->size;
        block->next->next = block->next->next;
        if (block->next->next) {
            block->next->next->prev = block->next;
        }
        s->free_sz += block->next->size;
    }
}

// Allocate a block of size 'n'
void *arena_alloc(size_t n) {
    arena_state *s = (arena_state*)((char*)s + ARENA_BUF_ALIGN - sizeof(arena_state));
    void *p = arena_alloc_space(s, n + ARENA_BUF_ALIGN);
    if (p) {
        arena_block *block = (arena_block*)p - 1;
        block->size = n + ARENA_BUF_ALIGN;
        block->next = s->free;
        s->free = block;
        s->free_sz += n + ARENA_BUF_ALIGN;
    }
    return p;
}

// Free a block
void arena_free(void *p) {
    if (p) {
        arena_state *s = (arena_state*)((char*)s + ARENA_BUF_ALIGN - sizeof(arena_state));
        arena_block *block = (arena_block*)p - 1;
        arena_merge_free(s, p);
        block->next = s->free;
        s->free = block;
        s->free_sz += block->size;
    }
}

// Reallocate a block, returning the original pointer if possible
void *arena_realloc(void *p, size_t n) {
    if (p && (char*)p >= (char*)s->buf && (char*)p < (char*)s->buf + ARENA_BUF_SIZE) {
        arena_state *s = (arena_state*)((char*)s + ARENA_BUF_ALIGN - sizeof(arena_state));
        size_t old_sz = ((arena_block*)p - 1)->size;
        if (n <= old_sz) {
            // Try to shrink the block if possible
            if (n < old_sz) {
                arena_block *block = (arena_block*)p - 1;
                block->size = n + ARENA_BUF_ALIGN;
                s->free_sz += old_sz - n;
                return p;
            }
            // Otherwise, try to use the space saved by shrinking
            void *new_p = arena_alloc_space(s, n + ARENA_BUF_ALIGN);
            if (new_p) {
                memcpy(new_p, p, n);
                arena_free(p);
                return new_p;
            }
        }
        // If the block cannot be shrunk, try to grow into a free neighbour
        void *new_p = (char*)p + old_sz;
        if (new_p == (char*)s->alloc) {
            arena_block *block = (arena_block*)s->alloc;
            size_t new_sz = block->size + old_sz;
            if (new_sz <= ARENA_BUF_SIZE) {
                memcpy(new_p, p, n);
                arena_block *block = (arena_block*)p - 1;
                block->size = new_sz;
                s->alloc = (char*)s->alloc + new_sz;
                s->free_sz += new_sz - old_sz;
                return p;
            }
        }
    }
    // If reallocation is not possible, use malloc as a last resort
    return realloc(p, n);
}