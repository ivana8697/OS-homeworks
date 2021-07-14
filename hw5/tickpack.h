#ifndef TICKPACK_H
#define TICKPACK_H
#include <cpu.h>
#include <pic.h>

#define TEN_MIL_SEC 0x5000
//#define TEN_MIL_SEC 0x2e9a /*original count */
extern int total_time;
char *number;

/* Start ticking service that calls app_tick_callback every interval usecs */
void init_ticks();
int set_timer(IntHandler *function, int time, int runtime);

/* Shut down ticking service */
void shutdown_ticks(void);
int set_timer();
#endif
