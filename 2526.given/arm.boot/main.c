#include "main.h"

uint32_t stack_top;

volatile uint32_t head = 0;
volatile uint32_t tail = 0 ;
volatile uint8_t buffer[MAX_CHARS];

char line[80];
uint8_t offset=0;

uint32_t irq_timer_tick = 0;

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
  while(!ring_is_empty()){
    uint8_t byte = ring_get();
    console_echo(byte);
  }
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
    if(ring_is_empty()){
      wfi();
    }
    irqs_enable();
  }
}



