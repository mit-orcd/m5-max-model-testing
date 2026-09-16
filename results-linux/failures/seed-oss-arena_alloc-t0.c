#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct BlockHeader {
    size_t size;
    bool is_allocated;
} BlockHeader;

static void *arena_buf;
static size_t arena_total_size;
static const size_t ALIGN = _Alignof(max_align_t);

#define ALIGN_UP(x, a) (((uintptr_t)(x) + (a) - 1) / (a) * (a))

void arena_init(void *buf, size_t size) {
    arena_buf = buf;
    arena_total_size = size;
    *(size_t *)arena_buf = arena_total_size;
    BlockHeader *first = (BlockHeader *)((char *)arena_buf + sizeof(size_t));
    first->size = size - sizeof(size_t);
    first->is_allocated = false;
}

void *arena_alloc(size_t n) {
    if (n == 0) return NULL;

    char *buf_start = (char *)arena_buf;
    size_t size_storage = *(size_t *)buf_start;
    char *buf_end = buf_start + size_storage;
    BlockHeader *current = (BlockHeader *)(buf_start + sizeof(size_t));

    while ((char *)current < buf_end) {
        if (!current->is_allocated) {
            char *header_start = (char *)current;
            char *after_header = header_start + sizeof(BlockHeader);
            char *payload_start = (char *)ALIGN_UP((uintptr_t)after_header, ALIGN);
            size_t required = payload_start + n - header_start;

            if (current->size >= required) {
                current->is_allocated = true;
                if (current->size > required) {
                    BlockHeader *next = (BlockHeader *)(header_start + required);
                    next->size = current->size - required;
                    next->is_allocated = false;
                    current->size = required;
                }
                return payload_start;
            }
        }
        current = (BlockHeader *)((char *)current + current->size);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL) return;

    char *buf_start = (char *)arena_buf;
    size_t size_storage = *(size_t *)buf_start;
    char *buf_end = buf_start + size_storage;
    BlockHeader *current = (BlockHeader *)(buf_start + sizeof(size_t));

    while ((char *)current < buf_end) {
        char *header_start = (char *)current;
        char *after_header = header_start + sizeof(BlockHeader);
        char *payload_start = (char *)ALIGN_UP((uintptr_t)after_header, ALIGN);

        if (payload_start == p) {
            current->is_allocated = false;

            char *next_header_start = header_start + current->size;
            if (next_header_start < buf_end) {
                BlockHeader *next = (BlockHeader *)next_header_start;
                if (!next->is_allocated) current->size += next->size;
            }

            BlockHeader *prev = NULL;
            BlockHeader *tmp = (BlockHeader *)(buf_start + sizeof(size_t));
            while ((char *)tmp < buf_end) {
                if ((char *)tmp + tmp->size == header_start) {
                    prev = tmp;
                    break;
                }
                tmp = (BlockHeader *)((char *)tmp + tmp->size);
            }

            if (prev && !prev->is_allocated) prev->size += current->size;
            return;
        }
        current = (BlockHeader *)((char *)current + current->size);
    }
}

void *arena_realloc(void *p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }

    char *buf_start = (char *)arena_buf;
    size_t size_storage = *(size_t *)buf_start;
    char *buf_end = buf_start + size_storage;
    BlockHeader *old_block = NULL;
    char *old_payload = p;

    BlockHeader *current = (BlockHeader *)(buf_start + sizeof(size_t));
    while ((char *)current < buf_end) {
        char *header_start = (char *)current;
        char *after_header = header_start + sizeof(BlockHeader);
        char *payload_start = (char *)ALIGN_UP((uintptr_t)after_header, ALIGN);
        if (payload_start == old_payload) { old_block = current; break; }
        current = (BlockHeader *)((char *)current + current->size);
    }

    if (!old_block) return NULL;

    char *old_header = (char *)old_block;
    size_t old_payload_size = old_block->size - (old_payload - old_header);
    char *new_after_header = old_header + sizeof(BlockHeader);
    char *new_payload_start = (char *)ALIGN_UP((uintptr_t)new_after_header, ALIGN);
    size_t new_required = new_payload_start + n - old_header;

    char *next_header = old_header + old_block->size;
    if (next_header < buf_end) {
        BlockHeader *next = (BlockHeader *)next_header;
        if (!next->is_allocated && next->size >= (new_required - old_block->size)) {
            old_block->size += next->size;
            return old_payload;
        }
    }

    void *new_p = arena_alloc(n);
    if (!new_p) return NULL;

    size_t copy_size = (old_payload_size < n) ? old_payload_size : n;
    memcpy(new_p, p, copy_size);
    arena_free(p);
    return new_p;
}