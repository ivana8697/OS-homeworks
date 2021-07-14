/*********************************************************************
*
*       file:           tty.c
*       author:         betty o'neil
*
*       tty driver--device-specific routines for ttys 
*
*/
#include <stdio.h>  /* for kprintf prototype */
#include <serial.h>
#include <cpu.h>
#include <pic.h>
#include "ioconf.h"
#include "tty_public.h"
#include "tty.h"

struct tty ttytab[NTTYS];        /* software params/data for each SLU dev */

/* Record debug info in otherwise free memory between program and stack */
/* 0x300000 = 3M, the start of the last M of user memory on the SAPC */
#define DEBUG_AREA 0x300000
#define BUFLEN 20

char *debug_log_area = (char *)DEBUG_AREA;
char *debug_record;  /* current pointer into log area */ 

/* tell C about the assembler shell routines */
extern void irq3inthand(void), irq4inthand(void);

/* C part of interrupt handlers--specific names called by the assembler code */
extern void irq3inthandc(void), irq4inthandc(void); 

/* the common code for the two interrupt handlers */                           static void irqinthandc(int dev);

/* prototype for debug_log */ 
void debug_log(char *);

/*====================================================================
*
*       tty specific initialization routine for COM devices
*
*/
Queue inqueue;      //queue for read
Queue outqueue;     //queue for write
Queue echoqueue;    //queue for echo
Queue *inque = &inqueue;      //queue pointers
Queue *outque = &outqueue; 
Queue *echoque = &echoqueue;  

void ttyinit(int dev)
{
  int baseport;
  struct tty *tty;		/* ptr to tty software params/data block */

  debug_record = debug_log_area; /* clear debug log */
  baseport = devtab[dev].dvbaseport; /* pick up hardware addr */
  tty = (struct tty *)devtab[dev].dvdata; /* and software params struct */

  //initialize queues 
  init_queue(inque, MAXBUF);    //read
  init_queue(outque, MAXBUF);   //write
  init_queue(echoque, MAXBUF);  //echo

  if (baseport == COM1_BASE) {
      /* arm interrupts by installing int vec */
      set_intr_gate(COM1_IRQ+IRQ_TO_INT_N_SHIFT, &irq4inthand);
      pic_enable_irq(COM1_IRQ);
  } else if (baseport == COM2_BASE) {
      /* arm interrupts by installing int vec */
      set_intr_gate(COM2_IRQ+IRQ_TO_INT_N_SHIFT, &irq3inthand);
      pic_enable_irq(COM2_IRQ);
  } else {
      kprintf("Bad TTY device table entry, dev %d\n", dev);
      return;			/* give up */
  }
  tty->echoflag = 1;		/* default to echoing */
  //tty->rin = 0;               /* initialize indices */
  //tty->rout = 0;
  //tty->rnum = 0;              /* initialize counter */
  //tty->tin = 0;               /* initialize indices */
  //tty->tout = 0;
  //tty->tnum = 0;              /* initialize counter */


  /* enable interrupts on receiver */
  outpt(baseport+UART_IER, UART_IER_RDI); /* RDI = receiver data int */
}


/*====================================================================
*
*       Useful function when emptying/filling the read/write buffers
*
*/

#define min(x,y) (x < y ? x : y)


/*====================================================================
*
*       tty-specific read routine for TTY devices
*
*/

int ttyread(int dev, char *buf, int nchar)
{
  int baseport;
  struct tty *tty;
  int i;
  int saved_eflags;        /* old cpu control/status reg, so can restore it */

  baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

  //replace buffer with queue

  for (i = 0; i < nchar;) {
    saved_eflags = get_eflags();    //get current state 
    cli();                          //disable interrupts
    if(queuecount(inque) != 0) {    //if the input queue is not empty
      buf[i] = dequeue(inque);      //dequeue from input to buffer
      i++;                          //keep going
    }
      set_eflags(saved_eflags);      //go back to the saved state
  }
  return nchar;
}


/*====================================================================
*
*       tty-specific write routine for SAPC devices
*       (cs444: note that the buffer tbuf is superfluous in this code, but
*        it still gives you a hint as to what needs to be done for
*        the interrupt-driven case)
*
*/

int ttywrite(int dev, char *buf, int nchar)
{
  int baseport;
  struct tty *tty;
  int i, saved_eflags;
  //char ch;
  

  baseport = devtab[dev].dvbaseport; /* hardware addr from devtab */
  tty = (struct tty *)devtab[dev].dvdata;   /* software data for line */

  saved_eflags = get_eflags();                      //save current state
  cli();

  for (i = 0; (i < nchar); i++) {  
    if (enqueue(outque, buf[i])!= FULLQUE){
      outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);   //do interrput and output again
      set_eflags(saved_eflags);                                 //back to saved state
    }
  }

  for (; i < nchar ;) {
    cli();      
    if (enqueue(outque, buf[i]) != FULLQUE) {       //go until full queue
      i++;
      outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
    }
    set_eflags(saved_eflags); 
  }
  return nchar;
}

/*====================================================================
*
*       tty-specific control routine for TTY devices
*
*/

int ttycontrol(int dev, int fncode, int val)
{
  struct tty *this_tty = (struct tty *)(devtab[dev].dvdata);

  if (fncode == ECHOCONTROL)
    this_tty->echoflag = val;
  else return -1;
  return 0;
}



/*====================================================================
*
*       tty-specific interrupt routine for COM ports
*
*   Since interrupt handlers don't have parameters, we have two different
*   handlers.  However, all the common code has been placed in a helper 
*   function.
*/
  
void irq4inthandc()
{
  irqinthandc(TTY0);
}                              
  
void irq3inthandc()
{
  irqinthandc(TTY1);
}                              

void irqinthandc(int dev){  
  int ch, status;
  struct tty *tty = (struct tty *)(devtab[dev].dvdata);
  int baseport = devtab[dev].dvbaseport; /* hardware i/o port */;

  pic_end_int();                /* notify PIC that its part is done */
  debug_log("*");
  status = inpt(baseport+UART_LSR);

  if(status & UART_LSR_DR) {            //read chars
    ch = inpt(baseport + UART_RX);
      if (tty->echoflag){               //add to echo queue if echo flag on
        enqueue(echoque, ch);
        outpt(baseport+UART_IER, UART_IER_RDI | UART_IER_THRI);
      }
    enqueue(inque, ch);
  }

  if (status & UART_LSR_THRE) {         //write chars         
    if ((ch = dequeue(echoque)) != -1) {
      outpt(baseport+UART_TX, ch);
    }  
    if ((ch = dequeue(outque)) != -1) {
    outpt(baseport+UART_TX, ch);
    }
      outpt(baseport+UART_IER, UART_IER_RDI);
  }
}
/* append msg to memory log */
void debug_log(char *msg)
{
    strcpy(debug_record, msg);
    debug_record +=strlen(msg);
}

