#
# Makefile for misc devices that really don't fit anywhere else.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now inherited from the
# parent makes..
#

O_TARGET := misc.o

subdir-$(CONFIG_MRVL_GPIO)  += gpio
obj-$(CONFIG_MRVL_GPIO)     += gpio/mv_gpio.o

subdir-$(CONFIG_MRVL_WLAN)  += mvwlan
obj-$(CONFIG_MRVL_WLAN)     += mvwlan/mvwlan.o

subdir-$(CONFIG_MRVL_RGCFG) += ffnetdrv
obj-$(CONFIG_MRVL_RGCFG)    += ffnetdrv/unimac.o

include $(TOPDIR)/Rules.make

fastdep:
