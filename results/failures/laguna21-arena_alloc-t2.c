#include <stddef.h>
#include <stdint.h>

typedef struct Block {
    size_t size;
    int free;
    struct Block *next;
} Block;

static Block *arena_head = NULL;

void arena_init(void *buf, size_t size) {
    if (size < sizeof(Block) + sizeof(Block)) return;
    Block *b = (Block *)buf;
    b->size = size - sizeof(Block);
    b->free = 1;
    b->next = (Block *)((char *)buf + sizeof(Block) + sizeof(Block));
    
    Block *next = b->next;
    next->size = size - sizeof(Block) - sizeof(Block) - sizeof(Block);
    next->free = 0;
    next->next = NULL;
    
    arena_head = b;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t alignment = _Alignof(max_align_t);
    size_t req_size = n + sizeof(Block) + alignment - 1;
    if (req_size <= n) return NULL;
    
    Block *prev = NULL;
    Block *curr = arena_head;
    
    while (curr) {
        if (curr->free && curr->size >= n) {
            size_t space = curr->size - n;
            size_t align_mask = alignment - 1;
            size_t header_pad = (sizeof(Block) + align_mask) & ~align_mask;
            
            if (space >= header_pad + sizeof(Block)) {
                Block *new_free = (Block *)((char *)curr + sizeof(Block) + n + (header_pad & align_mask));
                new_free->size = space - header_pad;
                new_free->free = 1;
                new_free->next = curr->next;
                
                curr->size = n;
                curr->free = 0;
                curr->next = new_free;
                
                return (void *)((char *)curr + sizeof(Block) + (header_pad & align_mask));
            } else if (space >= sizeof(Block)) {
                curr->free = 0;
                return (void *)((char *)curr + sizeof(Block));
            }
        }
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    Block *curr = arena_head;
    while (curr) {
        void *block_start = (void *)((char *)curr + sizeof(Block));
        if (p == block_start || (char *)p > (char *)curr && (char *)p < (char *)curr + sizeof(Block) + curr->size) {
            Block *b = (Block *)((char *)p - sizeof(Block));
            b->free = 1;
            
            if (b->next && b->next->free) {
                b->size += sizeof(Block) + b->next->size;
                b->next = b->next->next;
            }
            
            if (curr != b && curr->next == b && b->free) {
                curr->size += sizeof(Block) + b->size;
                curr->next = b->next;
            }
            
            return;
        }
        curr = curr->next;
    }
}

void *arena_realloc(void *p, size_tn) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    
    Block *b = (Block *)((char *)p - sizeof(Block));
    size_t old_size = b->size;
    
    if (n <= old_size) {
        b->size = n;
        return p;
    }
    
    Block *next = b->next;
    if (next && next->free && old_size + sizeof(Block) + next->size >= n) {
        size_t grow = n - old_size;
        if (grow <= next->size) {
            b->size = n;
            b->next = next->next;
            return p;
        }
    }
    
    void *new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, old_size);
        arena_free(p);
    }
    return new_p;
}