#include <stddef.h>

typedef struct {
    int data[8];
    int head;
    int count;
} Ring;

void ring_init(Ring *r) {
    r->head =