# file:	 crt0.s
# name:	 joseph tam phui hui
# description:	 user startup module	
# date:	 3-3-97

.globl ustart1
.text
			
ustart1:
	call main1    #main1 in uprog1.c
	pushl %eax
	call exit	       

