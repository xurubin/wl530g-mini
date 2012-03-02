# Makefile for Linux 2.5.x kbuild / Linux 2.4

# export-objs was removed in Linux 2.5.60
# export-objs := hostap.o hostap_crypt.o

obj-m += hostap.o hostap_crypt.o hostap_crypt_wep.o
obj-m += hostap_cs.o
obj-m += hostap_plx.o
obj-m += hostap_pci.o

ifeq ($(TOPDIR)/Rules.make,$(wildcard $(TOPDIR)/Rules.make))
include $(TOPDIR)/Rules.make
endif

# Need to add pcmcia-cs include path in the beginning of include directories.
# CFLAGS would add it to the end, so this need to use CC..
ifneq ($(PCMCIA_PATH),)
CC += -I$(PCMCIA_PATH)/include
endif
