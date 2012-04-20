#
# milli_httpd Makefile
#
# Copyright (C) 2001 Broadcom Corporation
#
# $Id: Makefile,v 1.3 2004/01/05 05:39:17 Cheni_Shen Exp $
#

CFLAGS	+= -I. -I../shared/include -I/Marvell/Src/uClinux-dist/uClibc/include -Wall -Dlinux -DFLASH_EMULATOR -DLinux -DEMBED -DASUS #-DMIPS

LDFLAGS	+= -Wl,-elf2flt="" #-L$(TOP)/nvram

#CFLAGS	+= -I. -I$(TOP)/shared -I$(SRCBASE)/include -Wall -DFLASH_EMULATOR -DLinux -DMIPS -DASUS

CFLAGS	+= -g -DDEBUG
#CFLAGS	+= -s -O2
LDFLAGS	+= #-L$(TOP)/nvram -L$(TOP)/shared -L$(TOP)/netconf

vpath %.c $(TOP)/shared
vpath %.c $(TOP)/shared_common

all: httpd

clean:
	rm -f *.o *~ httpd

install: all
	install -D httpd $(INSTALLDIR)/usr/sbin/httpd

# httpd: httpd.o ej.o cgi.o web_ex.o ../shared/shutils.o common.o nvram_f.o nvmap.o utility.o ../shared/nvram_linux.o ../netconf/netconf_linux.o
httpd: httpd.o dispatcher.o handlers.o nvram.o
	$(CC) -o $@ $^ $(LDFLAGS) #../iptables/libiptc/libiptc.a -lnvram -lshared -lnetconf
