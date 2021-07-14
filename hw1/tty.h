/*********************************************************************
*
*       file:           tty.h
*       author:         betty o'neil
*
*       private header file for tty drivers
*       apps should not include this header
*/

#ifndef TTY_H
#define TTY_H
#include "queue/queue.h"

#define MAXBUF 6

struct tty {
  int echoflag;			/* echo chars in read */
 

};

extern struct tty ttytab[];

/* tty-specific device functions */
void ttyinit(int dev);
int ttyread(int dev, char *buf, int nchar);
int ttywrite(int dev, char *buf, int nchar);
int ttycontrol(int dev, int fncode, int val);

#endif 
