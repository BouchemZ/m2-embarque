#include "main.h"

uint32_t stack_top;

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

stream_t streams[2]; // 0 for uart0, 1 for uart1 but not used
uint32_t event_count = 0;

void rx_notify(void* cookie){
  uint8_t buffer[32];
  int count;
  while((count = stream_read(0, buffer, 32)) != 0){
    for (int i=0; i<count; i++){
      console_echo(buffer[i]);
    }
  }
  event_count++;
}

void stream_setup(){
    stream_set_read_listener(0, rx_notify, (void*)UART0);
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
  stream_setup();
  irqs_enable();
  for(;;){
    irqs_disable();
    if(ring_is_empty(&streams[0].rx_ring)){
      wfi();
    }
    irqs_enable();
  }
}



