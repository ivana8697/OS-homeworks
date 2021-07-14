.globl ustart3
.text

ustart3:
		call main3           #main3 in uprog3.c
    	pushl %eax
    	call exit
