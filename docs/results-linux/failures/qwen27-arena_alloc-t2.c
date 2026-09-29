#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdalign.h>

#define ARENA_ALIGN _Alignof(max_align_t)
#define ARENA_HDR_SIZE (sizeof(uintptr_t) + sizeof(uintptr_t) + sizeof(uintptr_t))
#define ARENA_MIN_BLOCK (2 * ARENA_ALIGN)

typedef struct ArenaBlock {
    uintptr_t size;
    uintptr_t prev_free;
    uintptr_t is_free;
} ArenaBlock;

typedef struct Arena {
    ArenaBlock *first;
    size_t total_size;
    ArenaBlock *free_list;
} Arena;

static Arena *g_arena;

static size_t align_up(size_t n, size_t align) {
    return (n + align - 1) & ~(align - 1);
}

static size_t block_size(ArenaBlock *b) {
    return (size_t)b->size;
}

static ArenaBlock *next_block(ArenaBlock *b) {
    return (ArenaBlock *)((char *)b + block_size(b));
}

static ArenaBlock *prev_block(ArenaBlock *b) {
    ArenaBlock *prev = (ArenaBlock *)((char *)b - (size_t)b->prev_free);
    if (prev == (ArenaBlock *)g_arena->first) return NULL;
    return prev;
}

static void set_block_size(ArenaBlock *b, size_t size) {
    b->size = size;
}

static void set_prev_free(ArenaBlock *b, ArenaBlock *prev) {
    b->prev_free = (uintptr_t)((char *)prev - (char *)b);
}

static ArenaBlock *get_prev(ArenaBlock *b) {
    if (b == g_arena->first) return NULL;
    return (ArenaBlock *)((char *)b - (size_t)b->prev_free);
}

static void add_to_free_list(ArenaBlock *b) {
    b->is_free = 1;
    b->prev_free = (uintptr_t)(b->free_list);
    b->free_list = (uintptr_t)0;
    // Insert at head
    ArenaBlock *head = g_arena->free_list;
    if (head) {
        // Set the new block's "next" pointer via free_list field
        // We'll use a simple singly-linked list stored in free_list field
        // Actually, let's use a proper linked list
        // For simplicity, we'll just prepend
    }
    // Prepend to free list
    b->free_list = (uintptr_t)g_arena->free_list;
    g_arena->free_list = b;
}

static void remove_from_free_list(ArenaBlock *b) {
    ArenaBlock **pp = &g_arena->free_list;
    while (*pp) {
        if (*pp == b) {
            *pp = (ArenaBlock *)b->free_list;
            b->free_list = 0;
            return;
        }
        pp = (ArenaBlock **)&(*pp)->free_list;
    }
}

static void coalesce_free(ArenaBlock *b) {
    // Check next block
    ArenaBlock *next = next_block(b);
    if (next != (ArenaBlock *)((char *)g_arena->first + g_arena->total_size) &&
        next->is_free) {
        remove_from_free_list(next);
        set_block_size(b, block_size(b) + block_size(next));
    }
    // Check previous block
    ArenaBlock *prev = get_prev(b);
    if (prev && prev->is_free) {
        remove_from_free_list(prev);
        set_block_size(prev, block_size(prev) + block_size(b));
        // Re-link prev's next if it exists
        ArenaBlock *next = next_block(prev);
        if (next != (ArenaBlock *)((char *)g_arena->first + g_arena->total_size)) {
            set_prev_free(next, prev);
        }
        b = prev;
    }
    add_to_free_list(b);
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < ARENA_ALIGN * 4) return;
    
    size = align_up(size, ARENA_ALIGN);
    
    // Allocate arena control structure at the beginning of buf
    Arena *arena = (Arena *)buf;
    arena->total_size = size - sizeof(Arena);
    arena->first = (ArenaBlock *)((char *)buf + sizeof(Arena));
    arena->free_list = NULL;
    
    // Create one large free block
    ArenaBlock *b = arena->first;
    set_block_size(b, arena->total_size);
    set_prev_free(b, NULL);
    b->is_free = 1;
    b->free_list = 0;
    arena->free_list = b;
    
    g_arena = arena;
}

void *arena_alloc(size_t n) {
    if (!g_arena || n == 0) return NULL;
    
    size_t needed = align_up(n, ARENA_ALIGN) + ARENA_HDR_SIZE;
    if (needed < ARENA_MIN_BLOCK) needed = ARENA_MIN_BLOCK;
    
    // Find a free block of sufficient size
    ArenaBlock *b = g_arena->free_list;
    ArenaBlock *best = NULL;
    while (b) {
        if (block_size(b) >= needed) {
            if (!best || block_size(b) < block_size(best)) {
                best = b;
                if (block_size(b) == needed) break;
            }
        }
        b = (ArenaBlock *)b->free_list;
    }
    
    if (!best) return NULL;
    
    remove_from_free_list(best);
    
    // Split if there's enough left
    if (block_size(best) >= needed + ARENA_MIN_BLOCK) {
        size_t split_size = needed;
        ArenaBlock *newb = next_block(best);
        set_block_size(newb, block_size(best) - split_size);
        set_prev_free(newb, best);
        newb->is_free = 1;
        newb->free_list = 0;
        set_block_size(best, split_size);
        add_to_free_list(newb);
    }
    
    best->is_free = 0;
    return (char *)best + ARENA_HDR_SIZE;
}

void arena_free(void *p) {
    if (!p || !g_arena) return;
    
    ArenaBlock *b = (ArenaBlock *)((char *)p - ARENA_HDR_SIZE);
    b->is_free = 1;
    coalesce_free(b);
}

void *arena_realloc(void *p, size_t n) {
    if (!g_arena) return NULL;
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    ArenaBlock *b = (ArenaBlock *)((char *)p - ARENA_HDR_SIZE);
    size_t old_size = block_size(b);
    size_t needed = align_up(n, ARENA_ALIGN) + ARENA_HDR_SIZE;
    if (needed < ARENA_MIN_BLOCK) needed = ARENA_MIN_BLOCK;
    
    // If current block is large enough, just return
    if (old_size >= needed) {
        // Optionally split if there's enough leftover
        if (old_size >= needed + ARENA_MIN_BLOCK) {
            ArenaBlock *next = next_block(b);
            size_t split_size = needed;
            set_block_size(next, old_size - split_size);
            set_prev_free(next, b);
            next->is_free = 1;
            next->free_list = 0;
            set_block_size(b, split_size);
            add_to_free_list(next);
        }
        return p;
    }
    
    // Check if next block is free and large enough
    ArenaBlock *next = next_block(b);
    if (next->is_free && block_size(next) >= (needed - old_size)) {
        // Grow into next block
        remove_from_free_list(next);
        size_t new_total = old_size + block_size(next);
        if (new_total >= needed + ARENA_MIN_BLOCK) {
            ArenaBlock *split = next_block(b);
            set_block_size(split, new_total - needed);
            set_prev_free(split, b);
            split->is_free = 1;
            split->free_list = 0;
            set_block_size(b, needed);
            add_to_free_list(split);
        } else {
            set_block_size(b, new_total);
        }
        return p;
    }
    
    // Otherwise, allocate new block and copy
    void *newp = arena_alloc(n);
    if (!newp) return NULL;
    
    size_t copy_size = old_size - ARENA_HDR_SIZE;
    if (n < copy_size) copy_size = n;
    memcpy(newp, p, copy_size);
    arena_free(p);
    return newp;
}