#include <stddef.h>

typedef struct block {
    size_t size;
    int free;
    struct block *next;
} block_t;

static char *arena_head;
static size_t arena_size;

void arena_init(void *buf, size_t size) {
    arena_head = (char *)buf;
    arena_size = size;

    block_t *first = (block_t *)buf;
    if (size >= sizeof(block_t)) {
        first->size = size - sizeof(block_t);
    } else {
        first->size = 0;
    }
    first->free = 1;
    first->next = 0;
}

void *arena_alloc(size_t n) {
    if (n == 0) {
        return 0;
    }

    size_t align = sizeof(void *) > _Alignof(max_align_t) ? sizeof(void *) : _Alignof(max_align_t);
    size_t need = n + align;

    block_t *prev = 0;
    block_t *cur = (block_t *)arena_head;

    while (cur) {
        size_t data_offset = (size_t)cur & (align - 1);
        size_t actual = data_offset ? (align - data_offset) + cur->size : cur->size;

        if (cur->free && actual >= need) {
            size_t remaining = actual - need;

            if (remaining >= sizeof(block_t) + align) {
                block_t *split = (block_t *)((char *)cur + sizeof(block_t) + need);
                split->size = remaining - sizeof(block_t);
                split->free = 1;
                split->next = cur->next;

                if (cur->size == need) {
                    cur->size = need;
                } else {
                    cur->size = need + (data_offset ? align - data_offset : 0);
                }
                cur->free = 0;
                if (prev) {
                    prev->next = split;
                } else {
                    arena_head = (char *)split;
                }
            } else {
                cur->size = need;
                cur->free = 0;
            }

            void *result = (void *)((char *)cur + sizeof(block_t) + data_offset);
            return result;
        }

        prev = cur;
        cur = cur->next;
    }

    return 0;
}

void arena_free(void *p) {
    if (!p) {
        return;
    }

    block_t *target = (block_t *)((char *)p - sizeof(block_t));
    target->free = 1;

    block_t *cur = (block_t *)arena_head;
    if (cur != target) {
        while (cur->next && cur->next != target) {
            cur = cur->next;
        }
    }

    if (cur->next == target) {
        cur->next = target->next;
    } else {
        return;
    }

    if (cur->free && cur->next) {
        cur->size += sizeof(block_t) + cur->next->size;
        cur->next = cur->next->next;
    }

    if (target->next && target->next->free) {
        target->size += sizeof(block_t) + target->next->size;
        target->next = target->next->next;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) {
        return arena_alloc(n);
    }
    if (n == 0) {
        arena_free(p);
        return 0;
    }

    block_t *cur = (block_t *)((char *)p - sizeof(block_t));

    if (cur->next && cur->next->free) {
        block_t *nb = cur->next;
        size_t align = sizeof(void *) > _Alignof(max_align_t) ? sizeof(void *) : _Alignof(max_align_t);
        size_t data_offset = (size_t)nb & (align - 1);
        size_t avail = data_offset ? (align - data_offset) + nb->size : nb->size;

        size_t need = n + align;
        if (avail >= need) {
            size_t remaining = avail - need;

            if (remaining >= sizeof(block_t) + align) {
                block_t *split = (block_t *)((char *)nb + sizeof(block_t) + need);
                split->size = remaining - sizeof(block_t);
                split->free = 1;
                split->next = nb->next;

                cur->size = need + (data_offset ? align - data_offset : 0);
                cur->free = 0;
                cur->next = split;
            } else {
                cur->size += avail;
                cur->free = 0;
                cur->next = nb->next;
            }

            return p;
        }
    }

    void *np = arena_alloc(n);
    if (np) {
        size_t copy = cur->size < n ? cur->size : n;
        __builtin_memcpy(np, p, copy);
        arena_free(p);
    }
    return np;
}