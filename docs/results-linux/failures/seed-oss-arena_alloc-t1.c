#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct ArenaInfo {
    size_t total_size;
} ArenaInfo;

typedef struct BlockHeader {
    size_t size;
    bool is_allocated;
} BlockHeader;

static void *arena_buf = NULL;
static size_t arena_total = 0;

static BlockHeader *find_block(void *p) {
    if (p == NULL || arena_buf == NULL) return NULL;
    ArenaInfo *info = (ArenaInfo *)arena_buf;
    BlockHeader *current = (BlockHeader *)((uintptr_t)arena_buf + sizeof(ArenaInfo));
    size_t align = _Alignof(max_align_t);

    while ((uintptr_t)current < (uintptr_t)arena_buf + info->total_size) {
        size_t P = (align - ((uintptr_t)current % align)) % align;
        void *data_start = (void *)((uintptr_t)current + P);
        size_t user_size = current->size - sizeof(BlockHeader) - P;
        void *data_end = (void *)((uintptr_t)data_start + user_size);

        if (data_start <= p && p < data_end) return current;
        current = (BlockHeader *)((uintptr_t)current + current->size);
    }
    return NULL;
}

void arena_init(void *buf, size_t size) {
    if (buf == NULL || size < sizeof(ArenaInfo) + sizeof(BlockHeader)) return;
    arena_buf = buf;
    arena_total = size;
    ArenaInfo *info = (ArenaInfo *)arena_buf;
    info->total_size = size;
    BlockHeader *first = (BlockHeader *)((uintptr_t)arena_buf + sizeof(ArenaInfo));
    first->size = size - sizeof(ArenaInfo);
    first->is_allocated = false;
}

void *arena_alloc(size_t n) {
    if (n == 0 || arena_buf == NULL) return NULL;
    size_t align = _Alignof(max_align_t);
    ArenaInfo *info = (ArenaInfo *)arena_buf;
    BlockHeader *current = (BlockHeader *)((uintptr_t)arena_buf + sizeof(ArenaInfo));

    while ((uintptr_t)current < (uintptr_t)arena_buf + info->total_size) {
        if (!current->is_allocated) {
            size_t P = (align - ((uintptr_t)current % align)) % align;
            size_t available = current->size - sizeof(BlockHeader) - P;
            if (available >= n) {
                size_t needed = sizeof(BlockHeader) + P + n;
                if (current->size - needed > 0) {
                    BlockHeader *next = (BlockHeader *)((uintptr_t)current + needed);
                    next->size = current->size - needed;
                    next->is_allocated = false;
                    current->size = needed;
                }
                current->is_allocated = true;
                return (void *)((uintptr_t)current + P);
            }
        }
        current = (BlockHeader *)((uintptr_t)current + current->size);
    }
    return NULL;
}

void arena_free(void *p) {
    if (p == NULL || arena_buf == NULL) return;
    BlockHeader *block = find_block(p);
    if (block == NULL) return;

    block->is_allocated = false;

    BlockHeader *next = (BlockHeader *)((uintptr_t)block + block->size);
    if ((uintptr_t)next < (uintptr_t)arena_buf + ((ArenaInfo *)arena_buf)->total_size && !next->is_allocated) {
        block->size += next->size;
    }

    BlockHeader *prev = NULL, *curr = (BlockHeader *)((uintptr_t)arena_buf + sizeof(ArenaInfo));
    while ((uintptr_t)curr < (uintptr_t)arena_buf + ((ArenaInfo *)arena_buf)->total_size) {
        BlockHeader *cand_next = (BlockHeader *)((uintptr_t)curr + curr->size);
        if (cand_next == block) { prev = curr; break; }
        curr = cand_next;
    }

    if (prev != NULL && !prev->is_allocated) {
        prev->size += block->size;
    }
}

void *arena_realloc(void *p, size_t n) {
    if (p == NULL) return arena_alloc(n);
    if (n == 0) { arena_free(p); return NULL; }
    if (arena_buf == NULL) return NULL;

    BlockHeader *block = find_block(p);
    if (block == NULL) return NULL;

    size_t align = _Alignof(max_align_t);
    size_t P = (align - ((uintptr_t)block % align)) % align;
    size_t old_user = block->size - sizeof(BlockHeader) - P;
    size_t new_needed = sizeof(BlockHeader) + P + n;
    size_t needed = new_needed - block->size;

    BlockHeader *next = (BlockHeader *)((uintptr_t)block + block->size);
    bool can_grow = ((uintptr_t)next < (uintptr_t)arena_buf + ((ArenaInfo *)arena_buf)->total_size) && !next->is_allocated && (next->size >= needed);

    if (can_grow) {
        block->size += next->size;
        return p;
    }

    void *new_p = arena_alloc(n);
    if (new_p == NULL) return NULL;

    size_t copy = (old_user < n) ? old_user : n;
    memcpy(new_p, p, copy);
    arena_free(p);
    return new_p;
}