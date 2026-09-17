```c
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ARENA_MAGIC 0xA7E7A7E7u
#define ARENA_FREE_MAGIC 0xF7EEF7EEu

typedef struct arena_block {
    size_t size;
    unsigned int magic;
    unsigned int next_free; /* index into free list, or 0xFFFFFFFF if allocated */
    struct arena_block *next; /* next block in memory */
    struct arena_block *prev; /* previous block in memory */
    struct arena_block *free_next; /* next in free list */
    struct arena_block *free_prev; /* prev in free list */
} arena_block_t;

typedef struct arena_header {
    unsigned int magic;
    unsigned int free_count;
    arena_block_t *first;
    arena_block_t *last;
    arena_block_t *free_list_head;
    arena_block_t *free_list_tail;
    size_t total_size;
    size_t used_size;
} arena_header_t;

static arena_header_t *arena_hdr;

static inline size_t align_up(size_t x, size_t align) {
    return (x + align - 1) & ~(align - 1);
}

static size_t header_size(void) {
    return align_up(sizeof(arena_header_t), _Alignof(max_align_t));
}

static size_t block_overhead(void) {
    return align_up(sizeof(arena_block_t), _Alignof(max_align_t));
}

static arena_block_t *make_block(void *ptr, size_t size) {
    arena_block_t *b = (arena_block_t *)ptr;
    b->size = size;
    b->magic = ARENA_MAGIC;
    b->next_free = 0xFFFFFFFFu;
    b->next = NULL;
    b->prev = NULL;
    b->free_next = NULL;
    b->free_prev = NULL;
    return b;
}

static void free_list_insert(arena_block_t *b) {
    if (arena_hdr->free_list_head == NULL) {
        arena_hdr->free_list_head = b;
        arena_hdr->free_list_tail = b;
        b->free_next = NULL;
        b->free_prev = NULL;
    } else {
        arena_block_t *tail = arena_hdr->free_list_tail;
        tail->free_next = b;
        b->free_prev = tail;
        b->free_next = NULL;
        arena_hdr->free_list_tail = b;
    }
    b->magic = ARENA_FREE_MAGIC;
    b->next_free = 0xFFFFFFFFu;
    arena_hdr->free_count++;
}

static void free_list_remove(arena_block_t *b) {
    if (b->free_prev) {
        b->free_prev->free_next = b->free_next;
    } else {
        arena_hdr->free_list_head = b->free_next;
    }
    if (b->free_next) {
        b->free_next->free_prev = b->free_prev;
    } else {
        arena_hdr->free_list_tail = b->free_prev;
    }
    b->free_next = NULL;
    b->free_prev = NULL;
    b->magic = ARENA_MAGIC;
    arena_hdr->free_count--;
}

static void merge_block(arena_block_t *a, arena_block_t *b) {
    /* a and b are adjacent, b immediately follows a in memory */
    a->size += b->size;
    a->next = b->next;
    if (b->next) {
        b->next->prev = a;
    } else {
        arena_hdr->last = a;
    }
}

static arena_block_t *split_block(arena_block_t *b, size_t new_size) {
    size_t overhead = block_overhead();
    if (new_size + overhead > b->size) {
        return NULL;
    }
    size_t remainder = b->size - new_size;
    if (remainder < overhead) {
        return NULL;
    }
    arena_block_t *n = make_block((char *)b + new_size, remainder);
    n->prev = b;
    n->next = b->next;
    if (b->next) {
        b->next->prev = n;
    } else {
        arena_hdr->last = n;
    }
    b->size = new_size;
    b->next = n;
    return n;
}

static void arena_free_internal(arena_block_t *b) {
    /* Check next block */
    if (b->next && b->next->magic == ARENA_FREE_MAGIC) {
        free_list_remove(b->next);
        merge_block(b, b->next);
    }
    /* Check prev block */
    if (b->prev && b->prev->magic == ARENA_FREE_MAGIC) {
        free_list_remove(b->prev);
        merge_block(b->prev, b);
        b = b->prev;
    }
    free_list_insert(b);
}

void arena_init(void *buf, size_t size) {
    arena_hdr = (arena_header_t *)buf;
    arena_hdr->magic = ARENA_MAGIC;
    arena_hdr->free_count = 0;
    arena_hdr->total_size = size;
    arena_hdr->used_size = 0;
    arena_hdr->free_list_head = NULL;
    arena_hdr->free_list_tail = NULL;

    size_t hsize = header_size();
    if (size < hsize + block_overhead()) {
        arena_hdr->first = NULL;
        arena_hdr->last = NULL;
        return;
    }

    char *data = (char *)buf + hsize;
    size_t data_size = size - hsize;
    arena_block_t *b = make_block(data, data_size);
    arena_hdr->first = b;
    arena_hdr->last = b;
    free_list_insert(b);
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;
    if (!arena_hdr || arena_hdr->magic != ARENA_MAGIC) return NULL;

    size_t overhead = block_overhead();
    size_t needed = align_up(n, _Alignof(max_align_t)) + overhead;

    /* Search free list for a block large enough */
    arena_block_t *b = arena_hdr->free_list_head;
    while (b) {
        if (b->size >= needed) {
            break;
        }
        b = b->free_next;
    }

    if (!b) {
        return NULL;
    }

    /* Remove from free list */
    free_list_remove(b);

    /* Split if possible */
    size_t data_size = align_up(n, _Alignof(max_align_t));
    if (b->size > data_size + overhead) {
        arena_block_t *newb = split_block(b, data_size + overhead);
        if (newb) {
            free_list_insert(newb);
        }
    }

    arena_hdr->used_size += b->size;
    return (char *)b + overhead;
}

void arena_free(void *p) {
    if (!p) return;
    if (!arena_hdr || arena_hdr->magic != ARENA_MAGIC) return;

    size_t overhead = block_overhead();
    arena_block_t *b = (arena_block_t *)((char *)p - overhead);
    if (b->magic != ARENA_MAGIC) return;

    arena_hdr->used_size -= b->size;
    arena_free_internal(b);
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return NULL;
    }
    if (!arena_hdr || arena_hdr->magic != ARENA_MAGIC) return NULL;