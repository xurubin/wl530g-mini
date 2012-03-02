#
# Makefile for the qd driver loadable module
#
TARGET = nvram_mod.o
OBJS = 	nvram.o \
	crc.o \
	nvram_linux.o 

CC=arm-elf-gcc
LD=arm-elf-ld

CFLAGS = -DMODULE -D__KERNEL__ -Dlinux -D__linux__ -Dunix -D__uClinux__ -DEMBED -DLINUX
CFLAGS += -I/Marvell/Src/uClinux-dist/linux-2.4.x/include -I/Marvell/Src/uClinux-dist/linux-2.4.x/include/linux
CFLAGS += -Wall -Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -fno-common
CFLAGS += -fno-common -pipe -fno-builtin -D__linux__ -DNO_MM -mapcs-32 -mshort-load-bytes
CFLAGS += -nostdinc -msoft-float 
CFLAGS += -I.
CFLAGS += -I/usr/local/lib/gcc-lib/arm-elf/2.95.3/include
#CFLAGS += -DDEBUG_QD -DMV_DEBUG
CFLAGS += -DNDEBUG
ROMFSINST = install
ROMFSDIR=/Marvell/Src/uClinux-dist/filesystem/target

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) -r $(OBJS) -o $(TARGET)

romfs:
	$(ROMFSINST) $(TARGET) $(ROMFSDIR)/lib/modules/$(TARGET)

clean:
	-rm -f $(TARGET) *.elf *.gdb *.o 

