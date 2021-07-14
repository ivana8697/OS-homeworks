#include <stdio.h>
#include "proc.h"
#include "sched.h"

extern void debug_log(char *msg);

void schedule() {

	PEntry *oldproc = curproc;
	int i;
	int run = 0;
	char buf[200] = "";

	cli();

	for (i = 1; i <= 3; i++) {
		if (proctab[i].p_status == RUN) {
			run = 1;
			curproc = &proctab[i];
			break;
		}
	}

	if (run == 0) {
		curproc = &proctab[0]; 
	}

	if (oldproc != curproc){

		print_process_switch(oldproc);
		/*if (oldproc->p_status == ZOMBIE) {
			sprintf(buf, "|(%dz-%d)", oldproc - proctab, curproc - proctab);
		} else if (oldproc->p_status == BLOCKED) {
			sprintf(buf, "|(%db-%d)", oldproc - proctab, curproc - proctab);
		} else {
			sprintf(buf, "|(%d-%d)", oldproc - proctab, curproc - proctab);
		}*/
	}	
    //if not empty store in debug log
	if (strlen(buf))
		debug_log(buf);

	//if empty append
	if (strlen(buf))
		debug_log(buf);

	asmswtch(oldproc, curproc);
    sti();
}

//Table to switch processes and print to debug log
void print_process_switch(PEntry *oldproc) {
	char buf[200] = "";

        //Checks the status, and depending on the status it will print the corresponding
        //tag value. z for zombie, b for blocked, and values for misc.
	if (oldproc->p_status == ZOMBIE) {
		sprintf(buf, "|(%dz-%d)", oldproc - proctab, curproc - proctab);
	} else if (oldproc->p_status == BLOCKED) {
		sprintf(buf, "|(%db-%d)", oldproc - proctab, curproc - proctab);
	} else {
		sprintf(buf, "|(%d-%d)", oldproc - proctab, curproc - proctab);
	}
        //stores in debug log if not empty
	if (strlen(buf))
		debug_log(buf);
}

void sleep(WaitCode event)
{

  char buf[200] = "";
  cli();
  //sprintf(buf, "sleep:%d", curproc - proctab);

  curproc->p_status = BLOCKED;
  curproc->p_waitcode = event;

  if (strlen(buf))
	debug_log(buf);

   schedule();
   sti();

}

void wakeup(WaitCode event)
{
    int i;
    cli(); 
    for(i = 1; i < NPROC; i++)
	if((proctab[i].p_status==BLOCKED) && (proctab[i].p_waitcode==event)){
	    proctab[i].p_status=RUN; 
	}

    sti();  

}
