/* file: tunix.c core kernel code */

#include <stdio.h>
#include <cpu.h>
#include <gates.h>
#include "tsyscall.h"
#include "tsystm.h"
#include "proc.h"
#include "sched.h"

extern IntHandler syscall; /* the assembler envelope routine    */
extern void finale(void);
/* kprintf is proto'd in stdio.h, but we don't need that for anything else */
//void kprintf(char *, ...);	

/* functions in this file */
//void debug_set_trap_gate(int n, IntHandler *inthand_addr, int debug);
//void set_trap_gate(int n, IntHandler *inthand_addr);
int sysexit(int);
void k_init(void);
void shutdown(void);
void syscallc( int user_eax, int devcode, char *buff , int bufflen);
void process0(void);

/* Record debug info in otherwise free memory between program and stack */
/* 0x300000 = 3M, the start of the last M of user memory on the SAPC */
#define DEBUG_AREA 0x300000
char *debug_log_area = (char *)DEBUG_AREA;
char *debug_record;  /* current pointer into log area */ 

#define MAX_CALL 6

int fin;
PEntry proctab[NPROC], *curproc;
extern void ustart1(void), ustart2(void), ustart3(void);

/* optional part: syscall dispatch table */
static  struct sysent {
       short   sy_narg;        /* total number of arguments */
       int     (*sy_call)(int, ...);   /* handler */
} sysent[MAX_CALL];

/****************************************************************************/
/* k_init: this function for the initialize  of the kernel system*/
/****************************************************************************/

void k_init(){
  debug_record = debug_log_area; /* clear debug log */
  cli();
  ioinit();            /* initialize the deivce */ 
  set_trap_gate(0x80, &syscall);   /* SET THE TRAP GATE*/
  /* Note: Could set these with array initializers */
  /* Need to cast function pointer type to keep ANSI C happy */
  sysent[TREAD].sy_call = (int (*)(int, ...))sysread;
  sysent[TWRITE].sy_call = (int (*)(int, ...))syswrite;
  sysent[TEXIT].sy_call = (int (*)(int, ...))sysexit;
 
  sysent[TEXIT].sy_narg = 1;    /* set the arg number of function */
  sysent[TREAD].sy_narg = 3;
  sysent[TIOCTL].sy_narg = 3;
  sysent[TWRITE].sy_narg = 3;

  sti();      /* user runs with interrupts on */
  processtable();
}

void processtable() {
  int i = 0;
  fin = 0;
  //curproc = (PEntry *)&proctab;

  proctab[1].p_savedregs[SAVED_PC] = (int) &ustart1;
  proctab[2].p_savedregs[SAVED_PC] = (int) &ustart2;
  proctab[3].p_savedregs[SAVED_PC] = (int) &ustart3;
  proctab[1].p_savedregs[SAVED_ESP] = ESP1;
  proctab[2].p_savedregs[SAVED_ESP] = ESP2;
  proctab[3].p_savedregs[SAVED_ESP] = ESP3;

  for (i = 0; i < NPROC; i++) {
    proctab[i].p_savedregs[SAVED_EBP] = 0;
    proctab[i].p_savedregs[SAVED_EFLAGS] = (1 << 9); 
    proctab[i].p_status = RUN;
  }

  curproc = &proctab[0]; // Need to start from Kernel
  process0();
}

void process0() {

  int i = 1;
  for (; i < NPROC; i++) {
    while (proctab[i].p_status != ZOMBIE)
      schedule(i);
  }
}


/*//process 0
void process0(){
    int i;
    while ((proctab[1].p_status != ZOMBIE) || (proctab[2].p_status != ZOMBIE) || (proctab[3].p_status != ZOMBIE)) {
    //sti();
    //cli();
    schedule();
    //sti();
    }
        //wait for buffer to finish
        for (i=0; i< 1000000; i++)
    ;   
        
        //print debuf log and finish program
    shutdown();
}
*/
/* shut the system down */
void shutdown()
{
  kprintf("SHUTTING THE SYSTEM DOWN!\n");
  kprintf("Debug log from run:\n");
  kprintf("Marking kernel events as follows:\n");
  kprintf("  ^a   COM2 input interrupt, a received\n");
  kprintf("  ~    COM2 output interrupt, ordinary char output\n");
  kprintf("  ~e   COM2 output interrupt, echo output\n");
  kprintf("  ~s   COM2 output interrupt, shutdown TX ints\n");
  kprintf("%s", debug_log_area);	/* the debug log from memory */
  kprintf("\nLEAVE KERNEL!\n\n");
  finale();		/* trap to Tutor */
}

/****************************************************************************/
/* syscallc: this function for the C part of the 0x80 trap handler          */
/* OK to just switch on the system call number here                         */
/* By putting the return value of syswrite, etc. in user_eax, it gets       */
/* popped back in sysentry.s and returned to user in eax                    */
/****************************************************************************/

void syscallc( int user_eax, int devcode, char *buff , int bufflen)
{
  int nargs;
  int syscall_no = user_eax;

  switch(nargs = sysent[syscall_no].sy_narg)
    {
    case 1:         /* 1-argument system call */
	user_eax = sysent[syscall_no].sy_call(devcode);   /* sysexit */
	break;
    case 3:         /* 3-arg system call: calls sysread or syswrite */
	user_eax = sysent[syscall_no].sy_call(devcode,buff,bufflen); 
	break;
    default: kprintf("bad # syscall args %d, syscall #%d\n",
		     nargs, syscall_no);
    }
} 

/****************************************************************************/
/* sysexit: this function for the exit syscall fuction */
/****************************************************************************/

int sysexit(int exit_code){ 
  kprintf("\n EXIT CODE IS %d\n", exit_code);
  curproc->p_status = ZOMBIE;
  curproc->p_exitval = exit_code;
  if (++fin < 3) {
    curproc = (PEntry *)&proctab;
    process0();
  }
  else
    shutdown();
	return 0;    /* never happens, but keeps gcc happy */
}

/****************************************************************************/
/* set_trap_gate: this function for setting the trap gate */
/****************************************************************************/
extern void locate_idt(unsigned int *limitp, char ** idtp);

/*{
  debug_set_trap_gate(n, inthand_addr, 0);
}
*/
/* write the nth idt descriptor as a trap gate to inthand_addr */
/*void debug_set_trap_gate(int n, IntHandler *inthand_addr, int debug)
{
  char *idt_addr;
  Gate_descriptor *idt, *desc;
  unsigned int limit = 0;

  if (debug)
    kprintf("Calling locate_idt to do sidt instruction...\n");
  locate_idt(&limit,&idt_addr);
  idt = (Gate_descriptor *)(idt_addr - KERNEL_BASE_LA);
  if (debug)
    kprintf("Found idt at %x, lim %x\n",idt, limit);
  desc = &idt[n];               
  if (debug)
    kprintf("Filling in desc at %x with addr %x\n",(unsigned int)desc,
           (unsigned int)inthand_addr);
  desc->selector = KERNEL_CS;   
  desc->addr_hi = ((unsigned int)inthand_addr)>>16; 
  desc->addr_lo = ((unsigned int)inthand_addr)&0xffff;
  desc->flags = GATE_P|GATE_DPL_KERNEL|GATE_TRAPGATE;
  desc->zero = 0;
}*/

/* append msg to memory log */
void debug_log(char *msg)
{
    strcpy(debug_record, msg);
    debug_record +=strlen(msg);
}


