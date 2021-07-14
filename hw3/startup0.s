# file:	 startup0.s
# asm startup file
# very first module in load

        .globl _start, finale
.text
_start:  movl $0x3ffff0, %esp  # set initial kernel stack
	   call _startupc         # call C startup

finale: int $3                #  return to Tutor
	


