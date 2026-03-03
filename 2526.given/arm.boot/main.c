#include "main.h"

uint32_t stack_top;

ring_t ring = {
    .head = 0,
    .tail = 0,
    .buffer = {0}
};

char line[80];
uint8_t offset=0;

volatile uint32_t irq_timer_tick = 0;

char history[HISTORY_SIZE][80];
int history_idx = -1;

/* Forward declaration of kprintf from kprintf.c */
void kprintf(const char *fmt, ...);

static size_t strlen(const char *s) {
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

/*
 * Define ECHO_ZZZ to have a periodic reminder that this code is polling
 * the UART, actively. This means the processor is running continuously.
 * Polling is of course not the way to go, the processor should halt in
 * a low-power state and wake-up only to handle an interrupt from the UART.
 * But this would require setting up interrupts...
 */
#define ECHO_ZZZ

void panic() {
  while (1)
	  ;
}

// faire une boucle de 1sec
void wait(){
	for (int i=0; i<1000000; i++){
	}
}

void check_memory() {
  void *max = (void*)MEMORY;
  void *addr = &stack_top;
  if (addr >= max)
    panic();
}

void process_ring(){
  while(!ring_is_empty(&ring)){
    uint8_t byte = ring_get(&ring);
    console_echo(byte);
  }
}

stream_t streams[2]; // 0 for uart0, 1 for uart1 but not used
uint32_t event_count = 0;

void process_stream(int stream){
    // if there is a read listener and there are bytes to read, call the listener
    while (streams[stream].read_listener.callback != NULL && !ring_is_empty(&streams[stream].rx_ring)){
        streams[stream].read_listener.callback(streams[stream].read_listener.cookie);
    }
    // if there is a write listener and there is room to write, call the listener
    while (streams[stream].write_listener.callback != NULL && !ring_is_full(&streams[stream].tx_ring)){
        streams[stream].write_listener.callback(streams[stream].write_listener.cookie);
    }
}

void stream_setup(){
    stream_set_read_listener(0, console_echo, (void*)UART0);
    stream_set_write_listener(0, NULL, NULL);
}

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the startup.s file.
 */
void _start() {
  check_memory();
  console_init(da_vinci);
  irqs_setup();
  irqs_enable();
  for(;;){
    //kprintf("Tick: %u\n", irq_timer_tick);
    process_ring();
    irqs_disable();
    if(ring_is_empty(&ring)){
      wfi();
    }
    irqs_enable();
  }
}



