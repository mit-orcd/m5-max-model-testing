#include <stdalign.h>
#include <stddef.h>
#include <string.h>

typedef struct Block Block;
struct Block {
    size_t size;
    Block* next;
    char data[];
};

static Block* head;
static char* end;
static size_t buffer_size;

void arena_init(void* buf, size_t size) {
    buffer_size = size;
    head = (Block*)buf;
    head->size = size - offsetof(Block, data);
    head->next = NULL;
    end = (char*)buf + size;
}

void* arena_alloc(size_t n) {
    if (n == 0) return NULL;
    size_t align = alignof(max_align_t);
    size_t padding = (align - (offsetof(Block, data) % align)) % align;
    size_t total_size = offsetof(Block, data) + padding + n;
    if (total_size > buffer_size) return NULL;

    Block* prev = NULL;
    Block* curr = head;
    while (curr) {
        size_t curr_end = (char*)curr + offsetof(Block, data) + curr->size;
        if (curr->next) {
            size_t next_start = (char*)curr->next;
            if (curr_end + padding + n <= next_start) {
                size_t split_size = total_size;
                Block* split = (Block*)((char*)curr + offsetof(Block, data) + curr->size);
                split->size = curr->next->size - (split_size - curr->size);
                split->next = curr->next->next;
                curr->next->size = split_size - curr->size;
                curr->next->next = split->next;
                curr->size = split_size;
                return curr->data;
            }
        } else {
            if (curr_end + padding + n <= (char*)end) {
                curr->size += padding + n;
                return curr->data;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    return NULL;
}

void arena_free(void* p) {
    if (!p) return;
    Block* block = (Block*)((char*)p - offsetof(Block, data));
    Block* prev = NULL;
    Block* curr = head;
    while (curr && curr != block) {
        prev = curr;
        curr = curr->next;
    }
    if (!curr) return;

    if (prev && prev->next == curr) {
        prev->next = curr->next;
        prev->size += offsetof(Block, data) + curr->size;
        if (prev->next) {
            char* prev_end = (char*)prev + offsetof(Block, data) + prev->size;
            char* next_start = (char*)prev->next;
            if (prev_end == next_start) {
                prev->size += offsetof(Block, data) + prev->next->size;
                prev->next = prev->next->next;
            }
        }
    } else {
        head = curr->next;
        head->size += offsetof(Block, data) + curr->size;
        if (head->next) {
            char* head_end = (char*)head + offsetof(Block, data) + head->size;
            char* next_start = (char*)head->next;
            if (head_end == next_start) {
                head->size += offsetof(Block, data) + head->next->size;
                head->next = head->next->next;
            }
        }
    }
}

void* arena_realloc(void* p, size_t n) {
    if (!p) return arena_alloc(n);
    if (n == 0) {
        arena_free(p);
        return NULL;
    }

    Block* block = (Block*)((char*)p - offsetof(Block, data));
    size_t old_size = block->size - offsetof(Block, data);
    size_t min_size = (old_size < n) ? old_size : n;

    if (block->next) {
        size_t align = alignof(max_align_t);
        size_t padding = (align - (offsetof(Block, data) % align)) % align;
        size_t total_needed = offsetof(Block, data) + padding + n;
        size_t curr_end = (char*)block + offsetof(Block, data) + block->size;
        size_t next_start = (char*)block->next;
        if (curr_end + padding + n <= next_start) {
            size_t split_size = total_needed;
            Block* split = (Block*)((char*)block + offsetof(Block, data) + block->size);
            split->size = block->next->size - (split_size - block->size);
            split->next = block->next->next;
            block->next->size = split_size - block->size;
            block->next->next = split->next;
            block->size = split_size;
            return p;
        }
    }

    void* new_p = arena_alloc(n);
    if (new_p) {
        memcpy(new_p, p, min_size);
        arena_free(p);
    }
    return new_p;
}