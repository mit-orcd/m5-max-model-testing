#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct Block {
    size_t size;
    int free;
    struct Block *next;
} Block;

static Block *free_list = NULL;
static uint8_t *arena_buf = NULL;
static size_t arena_size = 0;

void arena_init(void *buf, size_t size) {
    if (!buf || size < sizeof(Block)) {
        free_list = NULL;
        arena_buf = NULL;
        arena_size = 0;
        return;
    }
    arena_buf = (uint8_t *)buf;
    arena_size = size;
    free_list = (Block *)arena_buf;
    free_list->size = size - sizeof(Block);
    free_list->free = 1;
    free_list->next = NULL;
}

static Block *find_block(void *p) {
    uint8_t *ptr = (uint8_t *)p;
    if (!ptr || ptr < arena_buf || ptr >= arena_buf + arena_size) return NULL;
    Block *b = (Block *)(ptr - sizeof(Block));
    if ((uint8_t *)b < arena_buf || (uint8_t *)(b + 1) > arena_buf + arena_size) return NULL;
    if (b->size == 0 || (uint8_t *)b + sizeof(Block) + b->size > arena_buf + arena_size) return NULL;
    return b;
}

static void *align_ptr(void *ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    size_t align = _Alignof(max_align_t);
    size_t mask = align - 1;
    if (align & (align - 1)) return NULL; // not power of two, but max_align_t alignment is power of two
    addr = (addr + mask) & ~mask;
    return (void *)addr;
}

static void coalesce(Block *b) {
    if (!b || !b->free) return;
    uint8_t *end = (uint8_t *)b + sizeof(Block) + b->size;
    Block *next = (Block *)end;
    if (end < arena_buf + arena_size && next->free) {
        b->size += sizeof(Block) + next->size;
        b->next = next->next;
    }
    Block *prev = NULL;
    for (Block *curr = free_list; curr; curr = curr->next) {
        if ((uint8_t *)curr + sizeof(Block) + curr->size == (uint8_t *)b) {
            prev = curr;
            break;
        }
    }
    if (prev && prev->free) {
        prev->size += sizeof(Block) + b->size;
        prev->next = b->next;
        b = prev;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t total = n + sizeof(Block);
    void *aligned = align_ptr(arena_buf);
    size_t offset = (uint8_t *)aligned - arena_buf;
    if (offset > arena_size) return NULL;
    size_t remaining = arena_size - offset;
    if (remaining < total) return NULL;

    Block *best = NULL;
    size_t best_diff = SIZE_MAX;
    Block *prev = NULL;
    Block *curr = free_list;

    while (curr) {
        if (curr->free && curr->size >= n) {
            size_t diff = curr->size - n;
            if (diff < best_diff) {
                best_diff = diff;
                best = curr;
                if (diff == 0) break;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    if (!best) return NULL;

    if (best->size >= n + sizeof(Block)) {
        Block *new_block = (Block *)((uint8_t *)best + sizeof(Block) + n);
        new_block->size = best->size - n - sizeof(Block);
        new_block->free = 1;
        new_block->next = best->next;
        best->size = n;
        best->next = new_block;
    }

    best->free = 0;
    if (best == free_list) {
        free_list = best->next;
    } else {
        if (prev) prev->next = best->next;
    }
    best->next = NULL;

    return (void *)((uint8_t *)best + sizeof(Block));
}

void arena_free(void *p) {
    if (!p) return;
    Block *b = find_block(p);
    if (!b || !b->free) return;
    b->free = 1;
    if (!free_list || b < free_list) {
        b->next = free_list;
        free_list = b;
    } else {
        Block *curr = free_list;
        while (curr->next && curr->next < b) curr = curr->next;
        b->next = curr->next;
        curr->next = b;
    }
    coalesce(b);
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    Block *b = find_block(p);
    if (!b || !b->free) return NULL;

    size_t old_size = b->size;
    if (n <= old_size) return p;

    size_t needed = n - old_size;
    uint8_t *end = (uint8_t *)b + sizeof(Block) + old_size;
    Block *next = (Block *)end;

    if (end < arena_buf + arena_size && next->free && next->size >= needed) {
        if (next->size >= needed + sizeof(Block)) {
            Block *new_next = (Block *)((uint8_t *)next + sizeof(Block) + needed);
            new_next->size = next->size - needed - sizeof(Block);
            new_next->free = 1;
            new_next->next = next->next;
            next = new_next;
        }
        b->size += needed + sizeof(Block);
        b->next = next->next;
        next->free = 0;
        if (next == free_list) {
            free_list = next->next;
        } else {
            Block *curr = free_list;
            while (curr && curr->next != next) curr = curr->next;
            if (curr) curr->next = next->next;
        }
        coalesce(b);
        return p;
    }

    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    size_t copy_size = old_size < n ? old_size : n;
    for (size_t i = 0; i < copy_size; i++) {
        ((uint8_t *)new_ptr)[i] = ((uint8_t *)p)[i];
    }
    arena_free(p);
    return new_ptr;
}