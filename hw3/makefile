
# CS444 hw3 makefile
# Usage: make tunix.lnx

# makefile for building C or assembly language programs for the
# Standalone 486. Also "make clean" to clean up non-source
# files in a directory
#
# system directories needed for compilers, libraries, header files--
#
#     March 1, 2020
#     Ron Cheung
#
SAPC_TOOLS = /home/cheungr/serl/tutor-linux
PC_LIB = $(SAPC_TOOLS)/libc
PC_INC = $(SAPC_TOOLS)/include

PC_CC    = gcc
PC_CFLAGS  = -gdwarf-2 -gstrict-dwarf -march=i586 -m32 -fno-builtin -fno-stack-protector -nostdlib     -c -Wall -I$(PC_INC)
PC_AS    = as --32
PC_NM    = nm
PC_LD    = ld -m elf_i386

FILES="README startup0.s startup1.c tunix.c tty.c asmswtch.s ulib.s sched.c io.c ioconf.c crt01.s crt02.s crt03.s uprog1.c uprog2.c uprog3.c sysentry.s makefile "

# File suffixes:
# .c	C source (often useful both for UNIX host and SAPC)
# .s 	assembly language source
# .lnx  executable image (bits as in memory), for SA PC (Linux a.out format)
# .syms text file of .lnx's symbols and their values (the "symbol table")
# Symbol file "syms"--for most recently built executable in directory

# PC executable--tell ld to start code at 0x100100, load special startup
# module, special PC C libraries--


all: tunix.lnx

# Object files for tiny unix OS

IO_OFILES = io.o tty.o ioconf.o queue.o 
KERNEL_OFILES = startup0.o startup.o tunix.o sysentry.o \
		asmswtch.o sched.o $(IO_OFILES)
USER_OFILES =   crt01.o crt02.o crt03.o \
		uprog1.o uprog2.o uprog3.o \
		ulib.o 

tunix.lnx: $(KERNEL_OFILES) $(USER_OFILES)
	$(PC_LD) -N -Ttext 100100 -o tunix.lnx \
	$(KERNEL_OFILES) $(USER_OFILES) $(PC_LIB)/libc.a
	rm -f syms;$(PC_NM) -n tunix.lnx>tunix.syms;ln -s tunix.syms syms

uprog1.o: uprog1.c tunistd.h tty_public.h 
	$(PC_CC) $(PC_CFLAGS) -c -o uprog1.o uprog1.c

uprog2.o: uprog2.c tunistd.h tty_public.h 
	$(PC_CC) $(PC_CFLAGS) -c -o uprog2.o uprog2.c

uprog3.o: uprog3.c tunistd.h tty_public.h 
	$(PC_CC) $(PC_CFLAGS) -c -o uprog3.o uprog3.c

io.o: io.c ioconf.h
	$(PC_CC) $(PC_CFLAGS) -c -o io.o io.c

tty.o: tty.c tty.h ioconf.h tty_public.h proc.h
	$(PC_CC) $(PC_CFLAGS) -c -o tty.o tty.c

ioconf.o: ioconf.c ioconf.h tty.h
	$(PC_CC) $(PC_CFLAGS) -c -o ioconf.o ioconf.c

crt01.o: crt01.s 
	$(PC_AS) -c -o crt01.o crt01.s

crt02.o: crt02.s 
	$(PC_AS) -c -o crt02.o crt02.s

crt03.o: crt03.s 
	$(PC_AS) -c -o crt03.o crt03.s

queue.o: queue/queue.c  queue/queue.h
	$(PC_CC) $(PC_CFLAGS) -c -o queue.o queue/queue.c

tunix.o: tunix.c tsyscall.h tsystm.h proc.h 
	$(PC_CC) $(PC_CFLAGS) -c -o tunix.o tunix.c

sched.o: sched.c proc.h  
	$(PC_CC) $(PC_CFLAGS) -c -o sched.o sched.c

sysentry.o: sysentry.s 
	$(PC_AS) -c -o sysentry.o sysentry.s

asmswtch.o: asmswtch.s 
	$(PC_AS) -c -o asmswtch.o asmswtch.s

ulib.o: ulib.s   
	$(PC_AS) -c -o ulib.o ulib.s

startup0.o: startup0.s
	$(PC_AS) -c -o startup0.o startup0.s

startup.o: startup1.c
	$(PC_CC) $(PC_CFLAGS) -c -o startup.o startup.c

clean:
	rm -f *.o core

# "make spotless" to remove (hopefully) everything except sources
#  use this after grading is done
spotless:
	rm -f *.o *syms *.lnx


