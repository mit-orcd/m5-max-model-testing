#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>

static void *arena_buf = NULL;
static size_t arena_size = 0;
static size_t arena_offset = 0;

typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header *next;
    struct block_header *prev;
} block_header_t;

static void align_offset(void) {
    size_t align = _Alignof(max_align_t);
    size_t remainder = arena_offset % align;
    if (remainder != 0) {
        arena_offset += (align - remainder);
    }
}

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_size = size;
    arena_offset = 0;
    
    if (size < sizeof(block_header_t)) {
        return;
    }

    align_offset();
    
    block_header_t *header = (block_header_t *)((char *)arena_buf + arena_offset);
    header->size = size - arena_offset - sizeof(block_header_t);
    header->is_free = 1;
    header->next = NULL;
    header->prev = NULL;
    
    arena_offset += sizeof(block_header_t) + header->size;
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_buf == NULL) {
        return NULL;
    }

    size_t align = _Alignof(max_align_t);
    size_t total_req = n + sizeof(block_header_t);
    size_t remainder = total_req % align;
    if (remainder != 0) {
        total_req += (align - remainder);
    }

    block_header_t *current = (block_header_t *)arena_buf;
    while (current != NULL) {
        if (current->is_free && current->size >= n) {
            size_t remaining = current->size - n;
            size_t header_size = sizeof(block_header_t);
            size_t align_rem = header_size % align;
            if (align_rem != 0) {
                header_size += (align - align_rem);
            }

            if (remaining >= header_size + n) {
                block_header_t *new_block = (block_header_t *)((char *)current + header_size + n);
                new_block->size = remaining - header_size - n;
                new_block->is_free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                if (current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = n;
                current->is_free = 0;
                return (void *)(current + 1);
            } else {
                current->is_free = 0;
                return (void *)(current + 1);
            }
        }
        current = current->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL || arena_buf == NULL) {
        return;
    }

    block_header_t *header = (block_header_t *)p - 1;
    if (header->is_free) {
        return;
    }

    header->is_free = 1;

    block_header_t *next = header->next;
    if (next && next->is_free) {
        header->size += sizeof(block_header_t) + next->size;
        header->next = next->next;
        if (next->next) {
            next->next->prev = header;
        }
    }

    block_header_t *prev = header->prev;
    if (prev && prev->is_free) {
        prev->size += sizeof(block_header_t) + header->size;
        prev->next = header->next;
        if (header->next) {
            header->next->prev = prev;
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

    block_header_t *header = (block_header_t *)p - 1;
    size_t current_size = header->size;
    size_t align = _Alignof(max_align_t);
    
    size_t total_req = n + sizeof(block_header_t);
    size_t remainder = total_req % align;
    if (remainder != 0) {
        total_req += (align - remainder);
    }

    if (current_size >= total_req) {
        size_t remaining = current_size - total_req;
        size_t header_size = sizeof(block_header_t);
        size_t align_rem = header_size % align;
        if (align_rem != 0) {
            header_size += (align - align_rem);
        }

        if (remaining >= header_size + n) {
            block_header_t *new_block = (block_header_t *)((char *)header + header_size + n);
            new_block->size = remaining - header_size - n;
            new_block->is_free = 1;
            new_block->next = header->next;
            new_block->prev = header;
            if (header->next) {
                header->next->prev = new_block;
            }
            header->next = new_block;
            header->size = n;
            return p;
        } else {
            return p;
        }
    }

    block_header_t *next = header->next;
    if (next && next->is_free) {
        size_t combined_size = current_size + sizeof(block_header_t) + next->size;
        if (combined_size >= total_req) {
            header->size = combined_size;
            header->next = next->next;
            if (next->next) {
                next->next->prev = header;
            }
            
            size_t remaining = header->size - total_req;
            size_t header_size = sizeof(block_header_t);
            size_t align_rem = header_size % align;
            if (align_rem != 0) {
                header_size += (align - align_rem);
            }

            if (remaining >= header_size + n) {
                block_header_t *new_block = (block_header_t *)((char *)header + header_size + n);
                new_block->size = remaining - header_size - n;
                new_block->is_free = 1;
                new_block->next = header->next;
                new_block->prev = header;
                if (header->next) {
                    header->next->prev = new_block;
                }
                header->next = new_block;
                header->size = n;
                return p;
            } else {
                header->size = n;
                return p;
            }
        }
    }

    void *new_ptr = arena_alloc(n);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    size_t copy_size = (current_size < n) ? current_size : n;
    if (copy_size > 0) {
        __builtin_memcpy(new_ptr, p, copy_size);
    }
    
    arena_free(p);
    return new_ptr;
}