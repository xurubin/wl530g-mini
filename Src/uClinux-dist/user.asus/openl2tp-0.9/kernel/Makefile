#############################################################################
# Copyright (C) 2004 Katalix Systems Ltd
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
#
#############################################################################

# Assumes x86 compiles at the moment - sorry.

ifndef KERNEL_SRCDIR
KERNEL_SRCDIR=/usr/src/linux
endif

# Use KERNEL_VERSION to determine whether to build the 2.4 or 2.6 module
ifeq ($(KERNEL_VERSION),2.4)
OBJ=		pppol2tp.o
IF_PPPOX.H=	if_pppox_v24.h
else
ifeq ($(KERNEL_VERSION),2.6)
OBJ=		pppol2tp.ko
IF_PPPOX.H=	if_pppox_v26.h
else
OBJ=		dummy
IF_PPPOX.H=	if_pppox_v24.h
endif
endif

.PHONY:	dummy clean test hfiles

ifneq ($(UML_TARGET),y)
# include/asm/mach-default is a 2.6 thing
ifeq ($(ARCH),ppc)
CFLAGS= -D__KERNEL__ -D__OPTIMIZE__ -I include -I $(KERNEL_SRCDIR)/include \
	-I $(KERNEL_SRCDIR)/arch/ppc \
	-I $(KERNEL_SRCDIR)/include/asm/mach-default \
	-Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing -ffreestanding \
	-fno-common -fomit-frame-pointer -pipe -ffixed-r2 -mmultiple -mstring \
	-Wall -DMODULE -DCONFIG_PPPOE_MODULE -DCONFIG_PPPOL2TP_MODULE
else
CFLAGS= -D__KERNEL__ -D__OPTIMIZE__ -I include -I $(KERNEL_SRCDIR)/include \
	-I $(KERNEL_SRCDIR)/include/asm/mach-default \
	-Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing \
	-fno-common -fomit-frame-pointer -pipe -mpreferred-stack-boundary=2 \
	-Wall -DMODULE -DCONFIG_PPPOE_MODULE -DCONFIG_PPPOL2TP_MODULE
endif
else
CFLAGS= -D__KERNEL__ -I include -I $(KERNEL_SRCDIR)/include \
	-I $(KERNEL_SRCDIR)/include/asm/mach-default \
	-I $(KERNEL_SRCDIR)/arch/um/include -Derrno=kernel_errno \
	-I $(KERNEL_SRCDIR)/arch/um/kernel/tt/include -I $(KERNEL_SRCDIR)/arch/um/kernel/skas/include \
	-Wstrict-prototypes -Wno-trigraphs -O2 -fno-strict-aliasing \
	-fno-common -fomit-frame-pointer -pipe -mpreferred-stack-boundary=2 \
	-U__i386__ -Ui386 -g -D__arch_um__ -DSUBARCH="i386" -D_LARGEFILE64_SOURCE \
	-Wall -DMODULE -DCONFIG_PPPOE_MODULE -DCONFIG_PPPOL2TP_MODULE
endif

all: 	hfiles $(OBJ)

pppol2tp.o: pppol2tp_v24.c
	$(CC) $(CFLAGS) -DKBUILD_BASENAME=$(basename $@) -c -o $@ $<

pppol2tp.ko: pppol2tp_v26.c
	@-$(RM) $(@:%.ko=%.tmp)
	$(CC) $(CFLAGS) -DKBUILD_BASENAME=$(basename $@) -c -o $(@:%.ko=%.tmp) $<
	@if [ -f $(@:%.ko=%.tmp) ]; then \
		$(KERNEL_SRCDIR)/scripts/mod/modpost -o pppol2tp.ksyms $(@:%.ko=%.tmp) ;\
		$(CC) $(CFLAGS) -DKBUILD_MODNAME=$(basename $@) -c -o pppol2tp.tmp.mod.o pppol2tp.tmp.mod.c ;\
		$(LD) -r -o pppol2tp.ko pppol2tp.tmp pppol2tp.tmp.mod.o ;\
	fi

hfiles:
	@if [ ! -e include/linux/if_pppox.h ]; then \
		cd include/linux ;\
		ln -s $(IF_PPPOX.H) if_pppox.h ;\
		cd ../.. ;\
	fi
ifeq ($(KERNEL_VERSION),2.4)
	@if [ ! -e include/linux/list.h ]; then \
		cd include/linux ;\
		ln -s list_v24.h list.h ;\
		cd ../.. ;\
	fi
	@if [ ! -e include/linux/hash.h ]; then \
		cd include/linux ;\
		ln -s hash_v24.h hash.h ;\
		cd ../.. ;\
	fi
endif

dummy:
	@echo
	@echo "*** KERNEL_VERSION not 2.4 or 2.6. Skipping kernel build. ***"
	@echo

install:

clean:
	$(MAKE) -C test $(MFLAGS) $@
	-$(RM) pppol2tp.o pppol2tp.ko include/linux/if_pppox.h include/linux/list.h include/linux/hash.h pppol2tp.tmp* pppol2tp.ksyms

test:
	$(MAKE) -C test $(MFLAGS) $@
