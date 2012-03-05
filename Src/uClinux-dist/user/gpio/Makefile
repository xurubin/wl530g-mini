LINUX_SRC	=../../linux-2.4.x

CFLAGS= -DMODULE -D__KERNEL__ -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -DLINUX -I$(LINUX_SRC)/include -I. -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common -pipe -fno-builtin -D__linux__ -DNO_MM -mapcs-32 -mshort-load-bytes -nostdinc -msoft-float -I/usr/local/lib/gcc-lib/arm-elf/2.95.3/include -DLIBERTAS

all: gpio.o

gpio.o: gpio.c
	$(CC) $(CFLAGS) -c $< -o $@

romfs:
	$(ROMFSINST) /lib/modules/gpio.o

clean:
	rm *.o

	
