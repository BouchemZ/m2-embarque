#include "main.h"


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

uint32_t stack_top;

void check_memory() {
  void *max = (void*)MEMORY;
  void *addr = &stack_top;
  if (addr >= max)
    panic();
}

// cursor loop
int  cursor_ind = 0;
char cursor_char[8]= { '|', '/', '-', '\\', '|', '/', '-', '\\', };

void blink_cursor(){
  uart_send_string(UART0,"\033[s");
  uart_send(UART0,cursor_char[cursor_ind]);
  uart_send_string(UART0,"\033[u");
  cursor_ind  = (cursor_ind == 7) ? 0 : cursor_ind + 1;
}

void does_nothing(){
  return;
}
void da_vinci(char* s){
  char res[79];
  int len = strlen(s);
  for (int i=0; i<len; i++){
    res[i] = s[len-1-i];
  }
  res[len] = '\0';
  kprintf("\nDa Vinci says:\n");
  kprintf("%s\n", res);
}

volatile uint32_t head;
volatile uint32_t tail;
volatile uint8_t buffer[MAX_CHARS];

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
    wfi();
  }
}



