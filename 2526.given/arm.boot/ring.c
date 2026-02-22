#include "ring.h"

bool_t ring_is_empty(){
    return head == tail;
}

bool_t ring_is_full(){
    int next = (head + 1) % MAX_CHARS;
    return next == tail;
}

// these definition of put and get imply that :
// you checked if the ring was full before put
// you checked if the ring was empty before a get

void ring_put(uint8_t byte){
    uint32_t next = (head + 1) % MAX_CHARS;
    buffer[head] = byte;
    head = next;
}

uint8_t ring_get(){
    uint8_t byte;
    uint32_t next = (tail+1)%MAX_CHARS;
    byte = buffer[tail];
    tail = next;
    return byte;
}