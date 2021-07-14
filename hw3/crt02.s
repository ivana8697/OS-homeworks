.globl ustart2
.text
			
ustart2:
		call main2                  #main2 in uprog2.c
        pushl %eax                  #push exit code on stack
        call exit                   