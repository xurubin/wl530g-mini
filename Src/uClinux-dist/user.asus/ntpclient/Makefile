#
# milli_httpd Makefile
#
# Copyright (C) 2001 Broadcom Corporation
#
# $Id: Makefile,v 1.2 2003/11/17 13:34:10 Cheni_Shen Exp $
#

CFLAGS	+= -I. -Wall -Dlinux 
#CFLAGS	+= -g -DDEBUG
#CFLAGS	+= -s -O2 
LDFLAGS	+= -Wl,-elf2flt="" #-L$(TOP)/nvram
INSTALLDIR = /root/broadcom/target
CC = arm-elf-gcc
STRIP = arm-elf-strip

vpath %.c $(TOP)/shared

all: ntpclient

clean:
	rm -f *.o *~ infosvr

install: all
# Neo
#	install -d $(INSTALLDIR)/usr/sbin
#	install httpd $(INSTALLDIR)/usr/sbin
#	$(STRIP) $(INSTALLDIR)/usr/sbin/httpd

	install -d $(INSTALLDIR)/sbin
	#$(STRIP) infosvr
	install infosvr $(INSTALLDIR)/sbin

ntpclient: ./ntpclient.o phaselock.o
	$(CC) -o $@ $^ $(LDFLAGS)

