#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define HEADER_SIZE 16  // Assuming 64-bit pointers and size_t

struct block {
    struct block* prev;
    struct block* next;
    size_t size;
    bool is_free;
};

struct arena_header {
    size_t arena_size;
    struct block dummy_head;
    struct block dummy_tail;
};

static struct arena_header* header = NULL;

void arena_init(void* buf, size_t size) {
    header = buf;
    struct arena_header* h = (struct arena_header*)buf;
    h->arena_size = size;
    h->dummy_head.prev = &h->dummy_tail;
    h->dummy_head.next = &h->dummy_tail;
    h->dummy_tail.prev = &h->dummy_head;
    h->dummy_tail.next = &h->dummy_head;
    
    struct block* initial_block = (struct block*)((char*)buf + sizeof(struct arena_header));
    initial_block->size = size - sizeof(struct arena_header);
    initial_block->is_free = true;
    initial_block->prev = &h->dummy_head;
    initial_block->next = h->dummy_head.next;
    h->dummy_head.next->prev = initial_block;
    h->dummy_head.next = initial_block;
}

void* arena_alloc(size_t n) {
    if (n == 0 || header == NULL) return NULL;
    
    const size_t alignment = _Alignof(max_align_t);
    size_t aligned_n = (n + alignment - 1) & ~(alignment - 1);
    size_t required_size = aligned_n + HEADER_SIZE;
    
    struct block* current = header->dummy_head.next;
    while (current != &header->dummy_tail) {
        if (current->is_free && current->size >= required_size) {
            current->is_free = false;
            if (current->size > required_size) {
                struct block* new_block = (struct block*)((char*)current + required_size);
                new_block->size = current->size - required_size;
                new_block->is_free = true;
                new_block->prev = current;
                new_block->next = current->next;
                current->next->prev = new_block;
                current->next = new_block;
                
                struct block* tmp = current->next;
                current->size = required_size;
                current = tmp;
            }
            return (void*)((char*)current + HEADER_SIZE);
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void* p) {
    if (p == NULL || header == NULL) return;
    
    struct block* block = (struct block*)((char*)p - HEADER_SIZE);
    if (!block->is_free) {
        block->is_free = true;
        if (block->prev != &header->dummy_head && block->prev->is_free) {
            struct block* prev_block = block->prev;
            prev_block->size += block->size;
            prev_block->next = block->next;
            block->next->prev = prev_block;
            block = prev_block;
        }
        if (block->next != &header->dummy_tail && block->next->is_free) {
            struct block* next_block = block->next;
            block->size += next_block->size;
            block->next = next_block->next;
            next_block->next->prev = block;
        }
        struct block* next_block = block->next;
        struct block* prev_block = block->prev;
        prev_block->next = next_block;
        next_block->prev = prev_block;
    }
}

void* arena_realloc(void* p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (header == NULL) return NULL;
    
    struct block* block = (struct block*)((char*)p - HEADER_SIZE);
    size_t old_size = block->size - HEADER_SIZE;
    if (old_size >= n) return p;
    
    const size_t alignment = _Alignof(max_align_t);
    size_t aligned_n = (n + alignment - 1) & ~(alignment - 1);
    size_t new_size = aligned_n + HEADER_SIZE;
    
    struct block* next_block = block->next;
    if (next_block != &header->dummy_tail && next_block->is_free) {
        size_t total_size = block->size + next_block->size;
        if (total_size >= new_size) {
            block->size = new_size;
            block->next = next_block->next;
            next_block->next->prev = block;
            return p;
        }
    }
    
    void* new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;
    memcpy(new_p, p, old_size);
    arena_free(p);
    return new_p;
}