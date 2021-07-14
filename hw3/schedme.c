#include <stdio.h>
#include <cpu.h>
#include "sched.h"
#include "proc.h"

extern void debug_log(char *msg);
extern void asmswtch(PEntry *oldpentryp, PEntry *newpentryp);

void schedule(int entry) {

	PEntry *oldproc = curproc;
	int zombiecount = 0;
	char buf[200];
	int i, first;

	cli();

	for(i = 0; i < NPROC; i++){
    	if(curproc == &proctab[i]){
      		first = i;
    	}
  	}
	
	for (i=1; i<NPROC; i++){
    	if (proctab[i].p_status == RUN) {
      		curproc=&proctab[i];
      		if(proctab[first].p_status == ZOMBIE){
        		sprintf(buf, "|(%dZ-%d)|",first, i); //for zombie
        		break;
      		} else {
        	  if(proctab[first].p_status == BLOCKED){  

          		sprintf(buf, "|(%dB-%d)|", first, i);  //for blocked
          		break;
        	  } else {
            		sprintf(buf, "|(%d-%d)|", first, i);
            		break;
          		}
      		}
    	}
    else if (proctab[i].p_status == ZOMBIE) {
      zombiecount++;
    }
  }
  if(zombiecount == NPROC -1){
    curproc = &proctab[0];
    if(proctab[first].p_status == ZOMBIE){
        sprintf(buf, "|(%dZ-0)|",first);
    } else {
      sprintf(buf, "|(%d-0)|", first);
    }

  }
  debug_log(buf);
  asmswtch(oldproc, curproc);
  return;
}

void sleep(WaitCode event)
{
  sti();
  cli();

  curproc->p_status = BLOCKED;
  curproc->p_waitcode = event;
  sti();
  //schedule();
  return;

}

void wakeup(WaitCode event)
{
    int i;
    sti();
	cli();
    for(i = 1; i < NPROC; i++){
		if(proctab[i].p_status==BLOCKED) 
		//&& (proctab[i].p_waitcode==event))
			proctab[i].p_status=RUN;
	}
	//return;
	sti();
}
