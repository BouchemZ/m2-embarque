#ifndef _STREAM_H_
#define _STREAM_H_ 

#include "stdint.h"
#include "stddef.h"
#include "main.h"
#include "ring.h"

#define STREAM_SIZE 512

typedef struct event {
    void (*callback)(void*);
    void* cookie;
} event_t;

typedef struct stream {
    event_t read_listener;
    event_t write_listener;
    //rx
    ring_t rx_ring;
    //tx
    ring_t tx_ring;
} stream_t;

/*
* Sets the read listener for the given stream.
* The given listener will be invoked, as part of its own event,
* whenever there are bytes to read from the given stream, passing
* back the given cookie.
*/
void stream_set_read_listener(int stream, void (*listener)(void*), void* cookie);

/*
* Sets the listener on the given stream to be called when
* there is available room to write bytes.
* The given listener will be invoked, as part of its own event,
* passing back the given cookie.
* Remove this listener when there is nothing to write by setting
* NULL as listener and cookie.
*/
void stream_set_write_listener(int stream, void (*listener)(void*), void* cookie);

/*
* Read up to "length" bytes in the given buffer.
* May read zero bytes, if none are available. Never blocks.
* Returns the number of bytes read; returns -1 if the end of stream has been reached.
*/
int stream_read(int stream, uint8_t* buffer, size_t length);
/*
* Write as many bytes as possible, from the given buffer,
* up to the given "length" bytes. Never blocks.
* Returns the number of bytes written; returns -1 if the end of stream has been reached.
*/
int stream_write(int stream, uint8_t* buffer, size_t length);


// stream l'id du stream à notifier
void stream_notify(int stream);

#endif /* _STREAM_H_ */
