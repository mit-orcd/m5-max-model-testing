#include <stddef.h>
#include <stdint.h>
#include <string.h>

static struct arena_header {
    size_t size;
    int free;
    struct arena_header *next;
    struct arena_header *prev;
} *arena_base = NULL;

static size_t arena_total_size = 0;

static struct arena_header *align_header(void *p) {
    uintptr_t addr = (uintptr_t)p;
    size_t align = _Alignof(max_align_t);
    if (addr % align != 0) {
        addr += align - (addr % align);
    }
    return (struct arena_header *)addr;
}

static void *header_to_ptr(struct arena_header *h) {
    return (void *)((uintptr_t)h + sizeof(struct arena_header));
}

static struct arena_header *ptr_to_header(void *p) {
    return (struct arena_header *)((uintptr_t)p - sizeof(struct arena_header));
}

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(struct arena_header)) {
        arena_base = NULL;
        arena_total_size = 0;
        return;
    }
    
    arena_base = align_header(buf);
    arena_total_size = (size_t)((uintptr_t)buf + size - (uintptr_t)arena_base);
    
    if (arena_total_size < sizeof(struct arena_header)) {
        arena_base = NULL;
        arena_total_size = 0;
        return;
    }
    
    arena_base->size = arena_total_size - sizeof(struct arena_header);
    arena_base->free = 1;
    arena_base->next = NULL;
    arena_base->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (arena_base == NULL || n == 0) {
        return NULL;
    }
    
    struct arena_header *current = arena_base;
    
    while (current != NULL) {
        if (current->free && current->size >= n) {
            if (current->size >= n + sizeof(struct arena_header) + _Alignof(max_align_t)) {
                size_t new_size = current->size - n - sizeof(struct arena_header);
                struct arena_header *new_block = (struct arena_header *)((uintptr_t)current + sizeof(struct arena_header) + n);
                new_block->size = new_size;
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if (current->next != NULL) {
                    current->next->prev = new_block;
                }
                
                current->size = n;
                current->next = new_block;
            }
            
            current->free = 0;
            return header_to_ptr(current);
        }
        current = current->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL || arena_base == NULL) {
        return;
    }
    
    struct arena_header *h = ptr_to_header(p);
    
    if ((uintptr_t)h < (uintptr_t)arena_base || (uintptr_t)h >= (uintptr_t)arena_base + arena_total_size) {
        return;
    }
    
    h->free = 1;
    
    struct arena_header *next = h->next;
    if (next != NULL && next->free) {
        h->size += sizeof(struct arena_header) + next->size;
        h->next = next->next;
        if (next->next != NULL) {
            next->next->prev = h;
        }
    }
    
    struct arena_header *prev = h->prev;
    if (prev != NULL && prev->free) {
        prev->size += sizeof(struct arena_header) + h->size;
        prev->next = h->next;
        if (h->next != NULL) {
            h->next->prev = prev;
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
    
    if (arena_base == NULL) {
        return NULL;
    }
    
    struct arena_header *h = ptr_to_header(p);
    
    if ((uintptr_t)h < (uintptr_t)arena_base || (uintptr_t)h >= (uintptr_t)arena_base + arena_total_size) {
        return NULL;
    }
    
    if (h->free) {
        return NULL;
    }
    
    struct arena_header *next = h->next;
    if (next != NULL && next->free && h->size + sizeof(struct arena_header) + next->size >= n) {
        h->size += sizeof(struct arena_header) + next->size;
        h->next = next->next;
        if (next->next != NULL) {
            next->next->prev = h;
        }
        return p;
    }
    
    void *new_ptr = arena_alloc(n);
    if (new_ptr != NULL) {
        size_t copy_size = h->size < n ? h->size : n;
        memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }
    
    return new_ptr;
}