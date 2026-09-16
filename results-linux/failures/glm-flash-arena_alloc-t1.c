#include <stddef.h>
#include <stdalign.h>
#include <string.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGNMENT_MASK (~(size_t)(ALIGNMENT - 1))
#define SIZE_MASK (~(size_t)(ALIGNMENT - 1))

typedef struct block {
    size_t size;
    struct block *next;
} block;

typedef struct {
    block *head;
    block *tail;
    block *free_head;
    block *free_tail;
    block *end;
} arena;

void arena_init(void *buf, size_t size) {
    arena *a = (arena *)buf;
    size_t usable = size - sizeof(arena);
    
    if (usable >= sizeof(block)) {
        a->head = (block *)((char *)buf + sizeof(arena));
        a->head->size = usable;
        a->head->next = NULL;
        
        a->tail = a->head;
        a->free_head = a->head;
        a->free_tail = a->head;
        a->end = (block *)((char *)buf + size);
    } else {
        a->head = NULL;
        a->tail = NULL;
        a->free_head = NULL;
        a->free_tail = NULL;
        a->end = NULL;
    }
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    arena *a = (arena *)((char *)NULL - sizeof(arena));
    
    size_t req = (n + ALIGNMENT - 1) & ALIGNMENT_MASK;
    
    block *curr = a->free_head;
    block *prev = NULL;
    
    while (curr) {
        if (curr->size >= req) {
            size_t remainder = curr->size - req;
            
            if (remainder > 0) {
                block *new_block = (block *)((char *)curr + req + sizeof(size_t));
                new_block->size = remainder;
                new_block->next = curr->next;
                curr->next = new_block;
                curr->size = req;
            }
            
            if (prev) {
                prev->next = curr->next;
                if (curr == a->free_tail) {
                    a->free_tail = prev;
                }
            } else {
                a->free_head = curr->next;
                if (curr == a->free_tail) {
                    a->free_tail = NULL;
                }
            }
            
            return (void *)(curr + 1);
        }
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;
    
    arena *a = (arena *)((char *)NULL - sizeof(arena));
    
    block *b = (block *)p - 1;
    
    b->next = a->free_head;
    a->free_head = b;
    if (a->free_tail == NULL) {
        a->free_tail = b;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    arena *a = (arena *)((char *)NULL - sizeof(arena));
    
    block *curr = (block *)p - 1;
    size_t current_size = curr->size;
    size_t req = (n + ALIGNMENT - 1) & ALIGNMENT_MASK;
    
    if (req <= current_size) {
        return p;
    }
    
    block *next = curr->next;
    
    if (next && ((char *)next - (char *)curr - current_size) >= sizeof(size_t)) {
        size_t free_space = next->size;
        size_t total_free = free_space + current_size;
        
        if (total_free >= req) {
            size_t new_size = total_free;
            size_t remainder = new_size - req;
            
            block *new_next = (block *)((char *)next + req);
            if (remainder > 0) {
                new_next->size = remainder;
                new_next->next = next->next;
                curr->next = new_next;
            } else {
                curr->next = next->next;
            }
            
            curr->size = req;
            
            if (next == a->free_head) {
                a->free_head = new_next;
            }
            
            return p;
        }
    }
    
    void *new_block = arena_alloc(n);
    if (new_block) {
        memcpy(new_block, p, current_size);
        arena_free(p);
    }
    return new_block;
}