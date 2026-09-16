#include <stddef.h>
#include <string.h>
#include <stdalign.h>

typedef struct {
    size_t size;      /* size of the payload (excluding header) */
    int free;         /* 1 if free, 0 if allocated */
    struct header *next;
    struct header *prev;
} header_t;

static header_t *arena_base = NULL;
static size_t arena_total_size = 0;

static size_t align_up(size_t n, size_t alignment) {
    return (n + alignment - 1) & ~(alignment - 1);
}

static size_t header_size(void) {
    return align_up(sizeof(header_t), alignof(max_align_t));
}

static header_t *next_block(header_t *h) {
    /* Calculate the next block by adding the current payload size to the current header */
    return (header_t *)((char *)h + header_size() + h->size);
}

static header_t *prev_block(header_t *h) {
    return h->prev;
}

void arena_init(void *buf, size_t size) {
    if (!buf || size < header_size() + alignof(max_align_t)) {
        arena_base = NULL;
        arena_total_size = 0;
        return;
    }

    arena_base = (header_t *)buf;
    arena_total_size = size;

    /* Create a single free block that spans the entire buffer */
    arena_base->size = size - header_size();
    arena_base->free = 1;
    arena_base->next = NULL;
    arena_base->prev = NULL;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return NULL;
    }

    if (!arena_base) {
        return NULL;
    }

    size_t aligned_n = align_up(n, alignof(max_align_t));
    size_t total_needed = header_size() + aligned_n;

    /* First-fit search */
    header_t *cur = arena_base;
    while (cur != NULL) {
        if (cur->free && cur->size >= aligned_n) {
            /* Check if we can split */
            if (cur->size >= aligned_n + header_size() + alignof(max_align_t)) {
                /* Split the block */
                header_t *new_block = (header_t *)((char *)cur + header_size() + aligned_n);
                
                cur->size = aligned_n;
                cur->free = 0;
                
                new_block->size = cur->next ? (cur->next - new_block) * sizeof(char) - header_size() : 0;
                /* Calculate new block size properly */
                new_block->size = (size_t)((char *)cur->next - (char *)new_block) - header_size();
                if (cur->next == NULL) {
                    new_block->size = arena_total_size - ((size_t)((char *)new_block - (char *)arena_base));
                    new_block->size -= header_size();
                }
                
                new_block->free = 1;
                new_block->next = cur->next;
                new_block->prev = cur;
                
                if (cur->next) {
                    cur->next->prev = new_block;
                }
                
                cur->next = new_block;
            } else {
                /* Use the entire block */
                cur->free = 0;
            }
            
            return (void *)((char *)cur + header_size());
        }
        cur = cur->next;
    }

    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) {
        return;
    }

    if (!arena_base) {
        return;
    }

    header_t *h = (header_t *)((char *)p - header_size());
    
    /* Bounds check: ensure the pointer is within the arena */
    if ((char *)h < (char *)arena_base || (char *)h >= (char *)arena_base + arena_total_size) {
        return;
    }

    if (h->free) {
        return; /* Already free */
    }

    h->free = 1;

    /* Merge with next block if it's free */
    header_t *next = h->next;
    if (next != NULL && next->free) {
        h->size += header_size() + next->size;
        h->next = next->next;
        if (next->next) {
            next->next->prev = h;
        }
    }

    /* Merge with previous block if it's free */
    header_t *prev = h->prev;
    if (prev != NULL && prev->free) {
        prev->size += header_size() + h->size;
        prev->next = h->next;
        if (h->next) {
            h->next->prev = prev;
        }
    }
}

void *arena_realloc(void *p, size_t n) {
    if (n == 0) {
        if (p) {
            arena_free(p);
        }
        return NULL;
    }

    if (p == NULL) {
        return arena_alloc(n);
    }

    if (!arena_base) {
        return NULL;
    }

    header_t *h = (header_t *)((char *)p - header_size());
    
    /* Bounds check */
    if ((char *)h < (char *)arena_base || (char *)h >= (char *)arena_base + arena_total_size) {
        return NULL;
    }

    if (h->free) {
        return NULL; /* Invalid pointer */
    }

    size_t aligned_n = align_up(n, alignof(max_align_t));

    /* If the current block is large enough, just return it */
    if (h->size >= aligned_n) {
        /* Check if we can shrink and split */
        if (h->size >= aligned_n + header_size() + alignof(max_align_t)) {
            header_t *new_block = (header_t *)((char *)h + header_size() + aligned_n);
            new_block->size = h->size - aligned_n - header_size();
            new_block->free = 1;
            new_block->next = h->next;
            new_block->prev = h;
            
            if (h->next) {
                h->next->prev = new_block;
            }
            
            h->size = aligned_n;
            h->next = new_block;
        }
        return p;
    }

    /* Try to grow into the next block if it's free and large enough */
    header_t *next = h->next;
    if (next != NULL && next->free) {
        size_t total_available = h->size + header_size() + next->size;
        if (total_available >= aligned_n) {
            /* Grow into next */
            h->size = aligned_n;
            
            /* Check if we can split the remainder */
            size_t remainder = total_available - aligned_n;
            if (remainder >= header_size() + alignof(max_align_t)) {
                header_t *new_block = (header_t *)((char *)h + header_size() + aligned_n);
                new_block->size = remainder - header_size();
                new_block->free = 1;
                new_block->next = next->next;
                new_block->prev = h;
                
                if (next->next) {
                    next->next->prev = new_block;
                }
                
                h->next = new_block;
            } else {
                /* Use the entire combined space */
                h->next = next->next;
                if (next->next) {
                    next->next->prev = h;
                }
            }
            
            return p;
        }
    }

    /* Try to grow into the previous block if it's free and large enough */
    header_t *prev = h->prev;
    if (prev != NULL && prev->free) {
        size_t total_available = prev->size + header_size() + h->size;
        if (total_available >= aligned_n) {
            /* Move data to prev block */
            size_t copy_size = h->size < aligned_n ? h->size : aligned_n;
            memcpy((char *)prev + header_size(), (char *)h + header_size(), copy_size);
            
            /* Remove h from the list */
            prev->size = aligned_n;
            prev->free = 0;
            prev->next = h->next;
            if (h->next) {
                h->next->prev = prev;
            }
            
            /* Check if we can split the remainder */
            size_t remainder = total_available - aligned_n;
            if (remainder >= header_size() + alignof(max_align_t)) {
                header_t *new_block = (header_t *)((char *)prev + header_size() + aligned_n);
                new_block->size = remainder - header_size();
                new_block->free = 1;
                new_block->next = prev->next;
                new_block->prev = prev;
                
                if (prev->next) {
                    prev->next->prev = new_block;
                }
                
                prev->next = new_block;
            }
            
            return (void *)((char *)prev + header_size());
        }
    }

    /* Need to allocate a new block and copy data */
    void *new_p = arena_alloc(n);
    if (new_p == NULL) {
        return NULL;
    }

    size_t copy_size = h->size < aligned_n ? h->size : aligned_n;
    memcpy(new_p, p, copy_size);
    
    arena_free(p);
    
    return new_p;
}