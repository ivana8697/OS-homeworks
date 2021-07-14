/* C startup file, called from startup0.s-- */

extern void clr_bss(void);
extern void init_devio(void);
extern void k_init(void);
void _startupc(void);

void _startupc()
{
  clr_bss();			/* clear BSS area (uninitialized data) */
  init_devio();			/* latch onto Tutor-supplied info, code */
  k_init();			/* execute user-supplied k_init */
}
