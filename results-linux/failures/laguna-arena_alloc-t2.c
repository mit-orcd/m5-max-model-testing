#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

typedef struct Block {
    struct Block *next;
    size_t size;
    int free;
} Block;

static void *arena_buffer;
static size_t arena_size;
static Block *arena_list;

void arena_init(void *buf, size_t size) {
    arena_buffer = buf;
    arena_size = size;
    arena_list = (Block *)buf;
    arena_list->next = NULL;
    arena_list->size = size - sizeof(Block);
    arena_list->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t align = _Alignof(max_align_t);
    size_t req_size = n + sizeof(Block);
    size_t aligned_req = (req_size + align - 1) & ~(align - 1);
    
    Block *current = arena_list;
    while (current) {
        if (current->free && current->size >= aligned_req) {
            if (current->size > aligned_req + sizeof(Block)) {
                Block *new_block = (Block *)((char *)current + aligned_req);
                new_block->next = current->next;
                new_block->size = current->size - aligned_req;
                new_block->free = 1;
                current->next = new_block;
                current->size = aligned_req;
            }
            current->free = 0;
            return (char *)current + sizeof(Block);
        }
        current = current->next;
    }
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    Block *block = (Block *)((char *)p - sizeof(Block));
    block->free = 1;
    
    Block *current = arena_list;
    while (current) {
        if (current->free && current->next && current->next->free) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        }
        current = current->next;
    }
    
    current = arena_list;
    while (current->next) {
        if (current->free && current->next->free) {
            current->size += sizeof(Block) + current->next->size;
            current->next = current->next->next;
        }
        current = current->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    Block *block = (Block *)((char *)p - sizeof(Block));
    size_t old_size = block->size;
    size_t align = _Alignof(max_align_t);
    size_t req_size = n + sizeof(Block);
    size_t aligned_req = (req_size + align - 1) & ~(align - 1);
    
    if (aligned_req <= old_size) return p;
    
    Block *next_block = block->next;
    if (next_block && next_block->free && 
        (old_size + sizeof(Block) + next_block->size) >= aligned_req) {
        block->size += sizeof(Block) + next_block->size;
        block->next = next_block->next;
        return p;
    }
    
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    
    size_t copy_size = old_size < n ? old_size : n;
    char *src = (char *)p;
    char *dst = (char *)new_ptr;
    for (size_t i = 0; i < copy_size; ++i) {
        dst[i] = src[i];
    }
    
    arena_free(p);
    return new_ptr;
}