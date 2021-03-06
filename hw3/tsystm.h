/*********************************************************************
*
*       file:           tsystm.h
*       author:         betty o'neil
*
*       kernel header for internal prototypes
*
*/

#ifndef TSYSTM_H
#define TSYSTM_H

#define ESP1 0x280000	//process1 stack start 
#define ESP2 0x290000	//process2 stack start
#define ESP3 0x2a0000	//process3 stack start

/* initialize io package*/
void ioinit(void);
/* read nchar bytes into buf from dev */
int sysread(int dev, char *buf, int nchar);
/* write nchar bytes from buf to dev */
int syswrite(int dev, char *buf, int nchar);
/* misc. device functions */
int syscontrol(int dev, int fncode, int val);

/* debugging support */
void debug_log(char *msg);

#endif
