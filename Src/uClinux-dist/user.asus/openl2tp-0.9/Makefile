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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307	USA 
#
#############################################################################

# BEGIN CONFIGURABLE SETTINGS
DESTDIR=/Marvell/Src/uClinux-dist/filesystem/target

# Define USE_DMALLOC to enable dmalloc memory debugging
# USE_DMALLOC=		y

# Define to include test code. This must be defined to run the
# regression tests
# L2TP_TEST=		y

# Define to compile in debug code. Also makes default trace flags
# enable all messages
# L2TP_DEBUG=		y

# Use asynchronous RPC requests where appropriate
# Affects only L2TP-PPP-IPPOOL interfaces, not management interfaces.
L2TP_USE_ASYNC_RPC=	y

# Build for UML environment?
# UML_TARGET=		y

# Point to kernel source tree
KERNEL_SRCDIR=/Marvell/Src/uClinux-dist/linux-2.4.x

# Base kernel version. 2.4 or 2.6. Determines which pppol2tp kernel
# source file is compiled.  May be empty to skip kernel build.
#KERNEL_VERSION=		2.4
KERNEL_VERSION=NONE

# Points to pppd install. 
# By default, pppd headers are assumed to be in /usr/include/pppd. but 
# can be pointed to a local pppd source tree if desired.
PPPD_VERSION=		2.4.2
PPPD_SRCDIR=		/Marvell/Src/uClinux-dist/user/pppd
# PPPD_LIBDIR=		/usr/lib/pppd/2.4.3

# Points to readline install root. READLINE_DIR should have lib/ & include/ subdirs
# If not defined, readline is assumed to be installed in the standard places that
# the compiler looks.
READLINE_DIR=		

# For cross-compiling
#CROSS_COMPILE=
CROSS_COMPILE=arm-elf-

# END CONFIGURABLE SETTINGS

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
INSTALL		= install

ifneq ($(READLINE_DIR),)
READLINE_LDFLAGS=	-L $(READLINE_DIR)/lib
READLINE_CFLAGS=	-I $(READLINE_DIR)/include
endif

export KERNEL_SRCDIR PPPD_VERSION PPPD_SRCDIR PPPD_LIBDIR KERNEL_VERSION READLINE_LDFLAGS READLINE_CFLAGS
export CROSS_COMPILE AS LD CC AR NM STRIP OBJCOPY OBJDUMP INSTALL UML_TARGET

# If KERNEL_VERSION=NONE, skip compiling the kernel stuff here. It is
# assumed that it is either built as part of the kernel or will be built
# later, perhaps by a distro's package installer.
ifneq ($(KERNEL_VERSION),NONE)
SUBDIRS=		kernel
endif
SUBDIRS+=		usl plugins pppd

PROGS.sbin=		openl2tpd
#PROGS.bin=		l2tpconfig

L2TP_RPC_STEM=		l2tp_rpc
L2TP_EVENT_RPC_STEM=	l2tp_event_rpc

RPC_FILES=		$(L2TP_RPC_STEM)_server.c $(L2TP_RPC_STEM)_client.c $(L2TP_RPC_STEM)_xdr.c \
			$(L2TP_EVENT_RPC_STEM)_server.c $(L2TP_EVENT_RPC_STEM)_client.c $(L2TP_EVENT_RPC_STEM)_xdr.c $(L2TP_EVENT_RPC_STEM).h

L2TPD_SRCS.c=		l2tp_main.c l2tp_common.c l2tp_avp.c l2tp_packet.c \
			l2tp_network.c l2tp_tunnel.c l2tp_peer.c l2tp_transport.c \
			l2tp_session.c l2tp_ppp.c \
			l2tp_plugin.c l2tp_test.c md5.c

L2TPD_SRCS.h=		l2tp_api.h l2tp_avp.h l2tp_private.h md5.h

ifeq ($(L2TP_TEST),y)
CPPFLAGS.l2tptest=	-DL2TP_TEST
endif

L2TPCONFIG_SRCS.c=	l2tp_config.c l2tp_common.c

L2TPD_SRCS.o=		$(L2TPD_SRCS.c:%.c=%.o) plugins/ppp_unix.o usl/usl_fd.o usl/usl_misc.o usl/usl_pid.o usl/usl_signal.o usl/usl_timer.o usl/usl_fsm.o
L2TPCONFIG_SRCS.o=	$(L2TPCONFIG_SRCS.c:%.c=%.o) $(L2TP_RPC_STEM)_client.o $(L2TP_RPC_STEM)_xdr.o $(L2TP_EVENT_RPC_STEM)_client.o $(L2TP_EVENT_RPC_STEM)_xdr.o

ifeq ($(USE_DMALLOC),y)
CPPFLAGS.dmalloc=	-DL2TP_DMALLOC
LIBS.dmalloc=		-ldmalloc
export USE_DMALLOC
endif

CPPFLAGS=		$(CPPFLAGS.l2tptest)
CFLAGS=-DWX54G -I. -Iusl -isystem $(KERNEL_SRCDIR)/include -Wall -s -O2 
LDFLAGS.l2tpd=-static -Wl,-elf2flt="-s 64000"

ifeq ($(L2TP_DEBUG),y)
CPPFLAGS+=		-DDEBUG
endif
ifeq ($(L2TP_USE_ASYNC_RPC),y)
CPPFLAGS+=		-DL2TP_ASYNC_RPC
endif

RPCGEN=			rpcgen
RPCGENFLAGS=		-N -M -C -L

.PHONY:			all clean distclean install subdirs-all wx54g tags

all:			generated-files subdirs-all $(PROGS.sbin) $(PROGS.bin)

wx54g:			clean all install

t: t.c
			$(CC) -o $@ $(CFLAGS) $< -Wl,-elf2flt

# Compile without -Wall because rpcgen-generated code is full of warnings
%_xdr.o:		%_xdr.c
			$(CC) -I. -MMD -s -c $(CPPFLAGS) $<

%_client.o:		%_client.c
			$(CC) -I. -MMD -s -c $(CPPFLAGS) $<

%_server.o:		%_server.c
			$(CC) -I. -MMD -s -c $(CPPFLAGS) $<

%_xdr.c:		%.x
			-$(RM) $@
			$(RPCGEN) $(RPCGENFLAGS) -c -o $@ $<

%_server.c:		%.x
			-$(RM) $@ $@.tmp
			$(RPCGEN) $(RPCGENFLAGS) -m -o $@.tmp $<
			cat $@.tmp | sed -e 's/switch (rqstp->rq_proc) {/if (l2tp_api_rpc_check_request(transp) < 0) return; switch (rqstp->rq_proc) {/' > $@

%_client.c:		%.x
			-$(RM) $@
			$(RPCGEN) $(RPCGENFLAGS) -l -o $@ $<

%.h:			%.x
			-$(RM) $@
			$(RPCGEN) $(RPCGENFLAGS) -h -o $@ $<

.PHONY:			generated-files plugins clean distclean cxref

generated-files:	$(RPC_FILES) bits_per_long.h

subdirs-all:
			@for d in $(SUBDIRS); do $(MAKE) -C $$d $(MFLAGS) EXTRA_CFLAGS="$(CPPFLAGS)" all; if [ $$? -ne 0 ]; then exit 1; fi; done

clean:
			@for d in $(SUBDIRS); do $(MAKE) -C $$d $(MFLAGS) $@; if [ $$? -ne 0 ]; then exit 1; fi; done
			-$(RM) $(L2TPD_SRCS.o) $(L2TPCONFIG_SRCS.o) $(PROGS.sbin) $(PROGS.bin) $(RPC_FILES)
			-$(RM) $(wildcard *.d) bits_per_long.h
			-$(RM) -r cxref
			-$(RM) $(wildcard l2tp_*rpc_*.tmp)
			$(RM) -f t

distclean:		clean
			-$(RM) TAGS

TAGS:
			@for d in $(SUBDIRS); do $(MAKE) -C $$d $(MFLAGS) $@; done
			etags -t $(wildcard *.c) $(wildcard *.h)

openl2tpd:		$(L2TPD_SRCS.o)
			$(CC) -o $@ $^ $(LDFLAGS.l2tpd) ../shared/nvram_linux.o
			ls -l $@

%.o:	%.c
			$(CC) -c $(CFLAGS) $< -o $@

bits_per_long.h:
			echo "/* Auto-generated from asm/types.h */" > $@
ifneq ($(UML_TARGET),y)
			grep BITS_PER_LONG $(KERNEL_SRCDIR)/include/asm/types.h >> $@
else
			grep BITS_PER_LONG $(KERNEL_SRCDIR)/include/asm/arch/types.h >> $@
endif

cxref:
			-mkdir cxref
			cxref $(wildcard *.[ch] usl/*.[ch]) \
				-R. -Ocxref -no-comments -xref-func -raw  \
				$(CPPFLAGS) \
				-I. -Iusl -Icli -I$(KERNEL_SRCDIR)/include > cxref/l2tp.raw

install:		all
			$(INSTALL) -d $(DESTDIR)/sbin
			$(INSTALL) $(PROGS.sbin) $(DESTDIR)/sbin

tags:
			ctags *.[ch] usl/*.[ch] plugins/*.[ch] pppd/*.[ch] *.x
			
sinclude		$(wildcard *.d) /dev/null
