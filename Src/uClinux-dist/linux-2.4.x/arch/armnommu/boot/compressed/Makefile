#
# linux/arch/arm/boot/compressed/Makefile
#
# create a compressed vmlinuz image from the original vmlinux
#
# Note! SYSTEM, ZTEXTADDR, ZBSSADDR and ZRELADDR are now exported
# from arch/arm/boot/Makefile
#

HEAD		 = head.o
OBJS		 = misc.o

CFLAGS		 = $(CPPFLAGS) -O3 -DSTDC_HEADERS $(CFLAGS_PROC) -msoft-float -D__linux__ -D__KERNEL__ -fno-strict-aliasing -fPIC

FONTC		 = $(TOPDIR)/drivers/video/font_acorn_8x8.c
ZLDFLAGS	 = -p -X -T $(LINUX).lds

#
# Architecture dependencies
#
ifeq ($(CONFIG_ARCH_ACORN),y)
OBJS		+= ll_char_wr.o font.o
ZLDFLAGS	+= -defsym params=$(PARAMS_PHYS)
endif

ifeq ($(CONFIG_ARCH_NETWINDER),y)
OBJS		+= head-netwinder.o
endif

ifeq ($(CONFIG_ARCH_SHARK),y)
OBJS		+= head-shark.o ofw-shark.o
endif

ifeq ($(CONFIG_ARCH_INTEGRATOR),y)
OBJS		+= head-integrator.o
endif

ifeq ($(CONFIG_ARCH_FTVPCI),y)
OBJS		+= head-ftvpci.o
endif

ifeq ($(CONFIG_ARCH_L7200),y)
OBJS		+= head-l7200.o
endif

ifeq ($(CONFIG_ARCH_CLPS7500),y)
HEAD		= head-clps7500.o
endif

ifeq ($(CONFIG_ARCH_P720T),y)
# Borrow this code from SA1100
OBJS		+= head-sa1100.o
endif

ifeq ($(CONFIG_ARCH_SA1100),y)
OBJS		+= head-sa1100.o setup-sa1100.o
ifeq ($(CONFIG_SA1100_NANOENGINE),y)
  OBJS += hw-bse.o
endif
endif

SEDFLAGS	= s/TEXT_START/$(ZTEXTADDR)/;s/LOAD_ADDR/$(ZRELADDR)/;

ifneq ($(ZBSSADDR),)
SEDFLAGS	+= s/BSS_START/$(ZBSSADDR)/
else
SEDFLAGS	+= s/BSS_START/ALIGN(4)/
endif

all:		$(LINUX)

$(LINUX):	$(HEAD) $(OBJS) piggy.rom $(LINUX).lds
		$(LD) $(ZLDFLAGS) $(HEAD) $(OBJS) -o $(LINUX)

$(HEAD): 	$(HEAD:.o=.S)
		$(CC) $(AFLAGS) -traditional -c $(HEAD:.o=.S)

piggy.xz:	$(SYSTEM) 
		$(OBJCOPY) -O binary -R .note -R .comment -S $(SYSTEM) piggy
		xz -c --check=crc32 --arm --lzma2=,dict=32MiB piggy >piggy.xz

piggy.rom:	piggy.xz
		python add_hdr.py $< $@

font.o:		$(FONTC)
		$(CC) $(CFLAGS) -Dstatic= -c -o $@ $(FONTC)

$(LINUX).lds:	$(LINUX).lds.in
		@sed "$(SEDFLAGS)" < $(LINUX).lds.in > $@

clean:;		rm -f $(LINUX) core piggy* $(LINUX).lds

.PHONY:	$(LINUX).lds clean

misc.o: misc.c $(TOPDIR)/include/asm/arch/uncompress.h

%.o: %.S
	$(CC) $(AFLAGS) $(EXTRA_AFLAGS) $(AFLAGS_$@) -c -o $@ $<
