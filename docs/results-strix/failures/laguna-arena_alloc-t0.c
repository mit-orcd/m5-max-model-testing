#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct Block {
    size_t size;
    struct Block *next;
    int free;
} Block;

static void *arena_ptr = NULL;
static size_t arena_size = 0;
static Block *free_list = NULL;

void arena_init(void *buf, size_t size) {
    arena_ptr = buf;
    arena_size = size;
    free_list = (Block *)buf;
    free_list->size = size - sizeof(Block);
    free_list->next = NULL;
    free_list->free = 1;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t align = _Alignof(max_align_t);
    size_t req_size = n + sizeof(Block) + align - 1;
    
    Block *prev = NULL;
    Block *curr = free_list;
    
    while (curr) {
        if (curr->free && curr->size >= req_size) {
            size_t block_size = curr->size;
            size_t header_size = sizeof(Block);
            size_t align_pad = (align - ((uintptr_t)(curr + 1) % align)) % align;
            
            Block *new_block = (Block *)((uintptr_t)curr + header_size + align_pad);
            new_block->size = block_size - header_size - align_pad;
            new_block->next = curr->next;
            new_block->free = 1;
            
            curr->size = header_size + align_pad;
            curr->next = new_block;
            curr->free = 0;
            
            return (void *)((uintptr_t)new_block + align_pad);
        }
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    Block *block = (Block *)((uintptr_t)p - sizeof(Block));
    block->free = 1;
    
    if (block->next && block->next->free) {
        block->size += sizeof(Block) + block->next->size;
        block->next = block->next->next;
    }
    
    Block *curr = free_list;
    while (curr) {
        if (curr->next == block) {
            if (curr->free) {
                curr->size += sizeof(Block) + block->size;
                curr->next = block->next;
            }
            return;
        }
        curr = curr->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    Block *block = (Block *)((uintptr_t)p - sizeof(Block));
    size_t old_size = block->size;
    
    if (block->next && block->next->free) {
        size_t total = old_size + sizeof(Block) + block->next->size;
        size_t align = _Alignof(max_align_t);
        size_t req_size = n + align - 1;
        
        if (total >= req_size) {
            block->next->free = 0;
            block->size = total;
            return p;
        }
    }
    
    void *new_ptr = arena_alloc(n);
    if (new_ptr) {
        size_t copy_size = old_size < n ? old_size : n;
        memcpy(new_ptr, p, copy_size);
        arena_free(p);
    }
    return new_ptr;
}