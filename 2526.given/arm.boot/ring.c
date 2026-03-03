#include "ring.h"

bool_t ring_is_empty(ring_t* ring){
    return ring->head == ring->tail;
}

bool_t ring_is_full(ring_t* ring){
    int next = (ring->head + 1) % MAX_CHARS;
    return next == ring->tail;
}

// these definition of put and get imply that :
// you checked if the ring was full before put
// you checked if the ring was empty before a get

void ring_put(ring_t* ring,uint8_t byte){
    uint32_t next = (ring->head + 1) % MAX_CHARS;
    ring->buffer[ring->head] = byte;
    ring->head = next;
}

uint8_t ring_get(ring_t* ring){
    uint8_t byte;
    uint32_t next = (ring->tail+1)%MAX_CHARS;
    byte = ring->buffer[ring->tail];
    ring->tail = next;
    return byte;
}