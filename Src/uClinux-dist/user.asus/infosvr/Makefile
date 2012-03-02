#
# milli_httpd Makefile
#
# Copyright (C) 2001 Broadcom Corporation
#
# $Id: Makefile,v 1.2 2003/11/17 13:34:10 Cheni_Shen Exp $
#
CFLAGS	+= -I. -I../shared/include -I$(TOP)/shared -I$(SRCBASE)/include -Wall -I. -Wall -Dlinux -DNO_PARALLEL #-DPRNINFO -DNO_PARALLEL

#CFLAGS	+= -g -DDEBUG
CFLAGS	+= -s -O2
LDFLAGS	+= -Wl,-elf2flt=""#-L$(TOP)/nvram -L$(TOP)/shared -L$(TOP)/netconf -lnvram -lshared -lnetconf

CC = arm-elf-gcc
STRIP = arm-elf-strip

vpath %.c $(TOP)/shared

all: infosvr

clean:
	rm -f *.o *~ infosvr

install: all
# Neo
#	install -d $(INSTALLDIR)/usr/sbin
#	install httpd $(INSTALLDIR)/usr/sbin
#	$(STRIP) $(INSTALLDIR)/usr/sbin/httpd

	install -d $(INSTALLDIR)/usr/sbin
	$(STRIP) infosvr
	install infosvr $(INSTALLDIR)/usr/sbin

infosvr: ./infosvr.o ./wlutils.o ./common.o ./packet.o ./service.o ./interface.o ../shared/shutils.o ../shared/nvram_linux.o
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
