#include <stddef.h>
#include <stdint.h>

#define ALIGN_MAX (sizeof(uint64_t))

typedef struct Block {
    size_t size;
    struct Block *next;
    struct Block *prev;
} Block;

typedef struct Arena {
    Block *head;
    Block *tail;
    size_t capacity;
} Arena;

#define HEADER_SIZE (sizeof(Block))
#define BLOCK_SIZE(size) (((size) + HEADER_SIZE + ALIGN_MAX - 1) & ~(ALIGN_MAX - 1))

static inline void *align_ptr(void *p) {
    return (void *)(((uintptr_t)p + ALIGN_MAX - 1) & ~(uintptr_t)(ALIGN_MAX - 1));
}

void arena_init(void *buf, size_t size) {
    Arena *a = (Arena *)buf;
    if (!buf || size < sizeof(Arena)) {
        a->head = NULL;
        a->tail = NULL;
        a->capacity = 0;
        return;
    }
    a->head = (Block *)(a + 1);
    a->tail = a->head;
    a->capacity = size - sizeof(Arena);
    a->head->size = a->capacity;
    a->head->next = NULL;
    a->head->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    Arena *a = (Arena *)((char *)align_ptr((char *)arena_init) - sizeof(Arena));
    Block *b = a->head;
    Block *prev = NULL;
    size_t alloc_size = BLOCK_SIZE(n);

    while (b) {
        if (b->size >= alloc_size) {
            size_t remainder = b->size - alloc_size;
            if (remainder > 0) {
                Block *new_block = (Block *)((char *)b + alloc_size);
                new_block->size = remainder;
                new_block->next = b->next;
                new_block->prev = b->loc;
                
                if (b->next) {
                    b->next->prev = new_block;
                } else {
                    a->tail = new_block;
                }
            }
            if (prev) {
                prev->next = b->next;
            } else {
                a->head = b->next;
            }
            b->next = NULL;
            b->prev = NULL;
            return (void *)(b + 1);
        }
        prev = b;
        b = b->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    Arena *a = (Arena *)((char *)align_ptr((char *)arena_init) - sizeof(Arena));
    Block *b = (Block *)((char *)p - HEADER_SIZE);
    b->next = a->head;
    b->prev = NULL;
    if (a->head) {
        a->head->prev = b;
    }
    a->head = b;
    a->tail = b;
    Block *current = b;
    while (current->next) {
        Block *next = current->next;
        if ((char *)current + current->size == (char *)next) {
            current->size += next->size;
            current->next = next->next;
            if (next->next) {
                next->next->prev = current;
            } else {
                a->tail = current;
            }
        } else {
            current = next;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!p) return arena_alloc(n);

    Arena *a = (Arena *)((char *)align_ptr((char *)arena_init) - sizeof(Arena));
    Block *b = (Block *)((char *)p - HEADER_SIZE);
    size_t current_size = b->size;
    size_t new_size = BLOCK_SIZE(n);
    size_t new_actual_size = n;

    if (new_size <= current_size) {
        return p;
    }

    if (b->next && (char *)b + current_size == (char *)b->next) {
        Block *next = b->next;
        if (next->size >= new_size - current_size) {
            next->size -= (new_size - current_size);
            b->size = new_size;
            return p;
        }
    }

    void *new_p = arena_alloc(n);
    if (new_p) {
        size_t copy_size = (current_size > new_actual_size) ? new_actual_size : current_size - HEADER_SIZE;
        __builtin_memcpy(new_p, p, copy_size);
        arena_free(p);
    }
    return new_p;
}