/* 
 * file   : tickpack.c 
 * purpose: hw5 timer routines 
 */
#include <stdio.h>
#include "tickpack.h"
#include <cpu.h>
#include <pic.h>
#include <timer.h>
#include "sched.h"
#include "tsystm.h"
#include "proc.h"

/* procedure prototype */
void set_timer_count(int);
void small_delay(void);
void irq0inthandc(void);
extern IntHandler irq0inthand;
void tick_handler(void);
int icount;

/* 1 sec = ~18 * downcount */
#define INTERVAL_MODULUS 18
#define COUNTS_PER_SEC 1193182 
#define COUNTS_PER_TICK (64*1024)

/* init_ticks: initiate the PIT
 * save the interval statically
 * Note: should be called with interrupts off
 */
void init_ticks()
{
  cli();			/* disable ints while setting them up */
  printf("Setting IDT interrupt gate descriptor for irq 0 (int n = 0x20)\n");
  set_intr_gate(TIMER0_IRQ+IRQ_TO_INT_N_SHIFT, &irq0inthand);
  printf("Commanding PIC to interrupt CPU for irq 0 (TIMER0_IRQ)\n");
  pic_enable_irq(TIMER0_IRQ);
  printf("Commanding timer to generate pulse train using this count\n");
  set_timer_count(0x0);	
  printf("Enabling interrupts in CPU\n");
  icount = 0;
  sti();   
}

/*
 * shutdown_tick: shut off interval timer
 * Note: should be called with interrupts off
 */
void shutdown_ticks()
{
  pic_disable_irq(TIMER0_IRQ);  /* disallow irq 0 ints to get to CPU */
}

/* 
 * Set up timer to count down from given count, then send a tick interrupt, 
 * over and over. A count of 0 sets max count, 65536 = 2**16 
 */

void set_timer_count(int count)
{
  outpt(TIMER_CNTRL_PORT, TIMER0|TIMER_SET_ALL|TIMER_MODE_RATEGEN);
  outpt(TIMER0_COUNT_PORT,count & 0xff); /* set LSB here */
  outpt(TIMER0_COUNT_PORT,count >> 8); /* and MSB here */
  small_delay();
}

/* 
 * about 10 us on a SAPC (400Mhz Pentium) 
 */
void small_delay(void)
{
  int i;
  
  for (i = 0;i < 1000;i++)
    ;
}

int set_timer(IntHandler *callback, int time, int runtime) {
  return 0;
}

void tick_handler() {

  number[0] = '*';
  number[2] = '\0';
  ++curproc->p_ticks;

  switch (curproc->p_number) {
  case 1:
    number[1] = '1';
    break;
  case 2:
    number[1] = '2';
    break;
  }
  
  if (curproc->p_number == 1 || curproc->p_number == 2)
    debug_log(number);
  
  if (--curproc->p_quantum == 0) {
    curproc->p_quantum = TEN_MIL_SEC;
#ifdef DEBUG
#endif
    scheduler(3);
  }
}

/* timer interrupt handler 
 */
void irq0inthandc(void)
{
    pic_end_int();	/* notify PIC with EOI command that its part is done */
  	putchar('*');		/* just show it for this little test */
  	tick_handler();
  	
}

