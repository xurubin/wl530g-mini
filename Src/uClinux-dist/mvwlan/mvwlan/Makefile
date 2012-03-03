#
# Makefile for the Marvell Libertas Wireless LAN driver loadable module
#
TARGET     = mvwlan.o
DBG_TARGET = mvwlan.gdb
OBJS       = mvWLAN.o \
	     mvWLAN_crypt.o \
	     mvWLAN_crypt_wep.o \
	     mvWLAN_crypt_tkip.o \
	     mvWLAN_crypt_ccmp.o \
	     mvWLAN_glue.o \
	     mvWLAN_ioctl.o \
	     mvWLAN_mgmt.o \
	     mvWLAN_proc.o \
	     mvWLAN_rx.o \
	     mvWLAN_tx.o \
	     mvWLAN_wds.o \
	     mvWLAN_rate.o \
	     mvWLAN_mfg.o

#CC=arm-elf-gcc
#LD=arm-elf-ld

XCFLAGS  = -I$(ROOTDIR)/$(LINUXDIR)/include \
	   -I../wlandrv/incl -I../wlandrv/src/hal/w8310
XCFLAGS += -DAP_SW -DFREE_IN_TXTASK -DCONDOR -DERP -DWDS_FEATURE -DSELF_CTS -DBOOSTER_MODE -DTURBO_SETUP -DAP_WPA2

ifndef CONFIG_LIBERTAS_AP32
XCFLAGS += -DPEREGRINE		\
		   -DAUTO_DETECT_MFG
endif

CFLAGS   = -DMODULE -D__KERNEL__ -DLINUX -Dlinux -D__linux__ -Dunix -DNO_MM \
	   -D__uClinux__ -DEMBED -Wall -Wstrict-prototypes \
	   -Wno-trigraphs -fshort-enums -fno-strict-aliasing -fno-common \
	   -fno-builtin -msoft-float -mapcs-32 -mshort-load-bytes -g -O2 \
	   -pipe $(XCFLAGS)


all: $(TARGET)

$(TARGET): $(OBJS) ../wlandrv/libwlan.a
	$(LD) -r $(OBJS) -o $(DBG_TARGET) -L./ -lmvwlan -L../wlandrv -lwlan -L/usr/local/lib/gcc-lib/arm-elf/2.95.3 -lgcc
	$(OBJCOPY) -g $(DBG_TARGET) $(TARGET)

clean:
	-rm -f $(TARGET) *.elf *.gdb *.o

romfs:
	$(ROMFSINST) $(TARGET) /lib/modules/$(TARGET)

convert:
	-find -name '*.[ch]' | xargs $(TOOLSDIR)/dos2unix -k