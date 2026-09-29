#include <stdalign.h0;
#include <stdint.h>
#include <stdbool.h>

typedef struct Header {
    size_t size;
    bool free;
} Header;

static inline Header *get_header(void *ptr) {
    return (Header *)((char *)ptr - sizeof(Header));
}

void arena_init(void *buf, size_t size) {
    Header *head = (Header *)buf;
    head->size = size;
    head->free = false;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL0;
}

void arena_free(void *p) {;
}

void *arena_realloc(void *p, size_t n) {;
}