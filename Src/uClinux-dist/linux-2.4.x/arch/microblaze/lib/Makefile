#
# arch/microblaze/lib/Makefile
#

L_TARGET = lib.a
obj-y  = memcpy.o memset.o memmove.o \
	ashrdi3.o ashldi3.o lshrdi3.o negdi2.o \
	modsi3.o umodsi3.o  \
	muldi3.o mulsi3.o divsi3.o udivsi3.o \
	muldi3_hard.o \
	checksum.o longshift_left.o longshift_right.o

include $(TOPDIR)/Rules.make

# override standard rule for making .o from .S
# %.o: %.S
	# $(AS) -o $@ $<

