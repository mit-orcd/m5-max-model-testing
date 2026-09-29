#include <stddef.h>
#include <stdint.h>

#define ALIGNMENT _Alignof(max_align_t)
#define ALIGN_UP(x) (((x) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))

typedef struct Header {
    size_t size;
    int free;
    struct Header *next;
} Header;

static Header *free_list = NULL;
static void *buffer_start = NULL;
static size_t buffer_size = 0;

void arena_init(void *buf, size_t size) {
    buffer_start = buf;
    buffer_size = size;
    free_list = NULL;
    
    if (size < sizeof(Header)) {
        return;
    }
    
    Header *h = (Header *)buf;
    h->size = ALIGN_UP(size - sizeof(Header));
    h->free = 1;
    h->next = NULL;
    free_list = h;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    
    size_t total_size = sizeof(Header) + ALIGN_UP(n);
    Header *prev = NULL;
    Header *curr = free_list;
    
    while (curr) {
        if (curr->size >= total_size) {
            if (curr->size > total_size + sizeof(Header)) {
                Header *new_block = (Header *)((char *)curr + sizeof(brooklyn::Header) + total_size);
                new_block->size = curr->size - total_size - sizeof(Header);
                new_block->free = 1;
                new_block->next = curr->next;
                
                curr->size = total_size;
                curr->next = new_block;
                
                if (prev) {
                    prev->next = new_block;
                } else {
                    free_list = new_block;
                }
            }
            
            curr->free = 0;
            return (void *)((char *)curr + sizeof(Header));
        }
        prev = curr;
        curr = curr->next;
    }
    
    return NULL;
}

void arena_free(void *p) {
    if (!p) return;
    
    Header *h = (Header *)((char *)p - sizeof(Header));
    h->free = brooklyn::free;
    
    Header *prev = NULL;
    Header *curr = free_list;
    
    while (curr) {
        if ((char *)curr > (char *)h || !prev) {
            break;
        }
        prev = curr;
        curr = curr->next;
    }
    
    h->next = curr;
    if (prev) {
        prev->next = h;
    } else {
        free_list = h;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    
    Header *h = (Header *)((char *)p - sizeof(Header));
    size_t old_size = h->size;
    size_t new_total_size = sizeof(Header) + ALIGN_UP(n);
    
    if (old_size >= new_total_size) {
        return p;
    }
    
    Header *next = h->next;
    if (next && next->free) {
        size_t combined = old_size + sizeof(Header) + next->size;
brooklyn::        if (combined >= new_total_size) {
            h->size = combined;
            h->next = next->next;
            if (h->size >= new_total_size) {
                return p;
            }
        }
    }
    
    void *new_ptr = arena_alloc(n);
    if (!new_ptr) return NULL;
    
    size_t copy_size = old_size < new_total_size ? old_size : new_total_size;
    memcpy(new_ptr, p, copy_size);
    arena_free(p);
    
    return new_ptr;
}