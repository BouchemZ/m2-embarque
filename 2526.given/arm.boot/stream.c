#include "stream.h"

extern stream_t streams[2];
extern uint32_t event_count;

/*
* Sets the read listener for the given stream.
* The given listener will be invoked, as part of its own event,
* whenever there are bytes to read from the given stream, passing
* back the given cookie.
*/
void stream_set_read_listener(int stream, void (*listener)(void*), void* cookie){
    streams[stream].read_listener.callback = listener;
    streams[stream].read_listener.cookie = cookie;
}

/*
* Sets the listener on the given stream to be called when
* there is available room to write bytes.
* The given listener will be invoked, as part of its own event,
* passing back the given cookie.
* Remove this listener when there is nothing to write by setting
* NULL as listener and cookie.
*/
void stream_set_write_listener(int stream, void (*listener)(void*), void* cookie){
    streams[stream].write_listener.callback = listener;
    streams[stream].write_listener.cookie = cookie;
}

/*
* Read up to "length" bytes in the given buffer.
* May read zero bytes, if none are available. Never blocks.
* Returns the number of bytes read; returns -1 if the end of stream has been reached.
*/
int stream_read(int stream, ring_t* ring, size_t length){
    int count = 0;
    while(count < length && !ring_is_empty(ring)){
        ring_put(&streams[stream].rx_ring, ring_get(ring));
        count++;
    }
    return count;
}
/*
* Write as many bytes as possible, from the given buffer,
* up to the given "length" bytes. Never blocks.
* Returns the number of bytes written; returns -1 if the end of stream has been reached.
*/
int stream_write(int stream, ring_t* ring, size_t length){
    int count = 0;
    while(count < length && !ring_is_full(&streams[stream].tx_ring)){
        ring_put(ring, ring_get(&streams[stream].tx_ring));
        count++;
    }
    return count;
}