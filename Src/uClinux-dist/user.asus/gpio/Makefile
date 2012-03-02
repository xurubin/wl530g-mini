
#CPU		= arm
CROSS_COMPILE	= arm-elf-
#DEBUG		= -DSMC_DEBUG=3

LINUX_SRC	=/Marvell/Src/uClinux-dist/linux-2.4.x

CFLAGS= -DMODULE -D__KERNEL__ -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -DLINUX -I$(LINUX_SRC)/include -I. -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -pipe -fno-builtin -D__linux__ -DNO_MM -mapcs-32 -mshort-load-bytes -nostdinc -msoft-float -I/usr/local/lib/gcc-lib/arm-elf/2.95.3/include -DLIBERTAS

#CFLAGS=-I$(LINUX_SRC)/include/asm/gcc -D__KERNEL__ -I$(LINUX_SRC)/include -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer -fno-strict-aliasing -G 0 -mno-abicalls -fno-pic -mcpu=r4600 -mips2 -Wa,--trap -m4710a0kern -pipe -I$(LINUX_SRC)/../../include  -I. -DMODULE -mlong-calls -fno-common -DLinux -DWL600 #-DRESET_TO_DEFAULT_POLLING

CC		:=$(shell if which $(CROSS_COMPILE)kgcc > /dev/null 2>&1; then echo $(CROSS_COMPILE)kgcc; else echo $(CROSS_COMPILE)gcc; fi)

all: gpio.o

install: all
	install -d $(INSTALLDIR)/lib/modules/2.4.5
	install gpio.o $(INSTALLDIR)/lib/modules/2.4.5
	
gpio.o: gpio.c
	$(CC) $(CFLAGS) -c gpio.c -o gpio.o

clean:
	rm -f *.o
