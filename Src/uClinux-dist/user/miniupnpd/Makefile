# $Id: Makefile.linux,v 1.62 2012/03/05 20:36:15 nanard Exp $
# MiniUPnP project
# (c) 2006-2011 Thomas Bernard
# http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
# Author : Thomas Bernard
# for use with GNU Make
#
# To install use :
# $ PREFIX=/dummyinstalldir make -f Makefile.linux install
# or :
# $ INSTALLPREFIX=/usr/local make -f Makefile.linux install
# or :
# $ make -f Makefile.linux install
#
# if your system hasn't iptables libiptc headers and binary correctly
# installed, you need to get iptables sources from http://netfilter.org/
# ./configure them and build them then miniupnpd will build using :
# $ IPTABLESPATH=/path/to/iptables-1.4.1 make -f Makefile.linux
#
CFLAGS += -Wall -O -D_GNU_SOURCE -g -DDEBUG -ansi
#CFLAGS ?= -Wall -Os -D_GNU_SOURCE -fno-strict-aliasing -Wstrict-prototypes


INSTALLPREFIX ?= $(PREFIX)/usr
SBININSTALLDIR = $(INSTALLPREFIX)/sbin
ETCINSTALLDIR = $(PREFIX)/etc/miniupnpd

BASEOBJS = miniupnpd.o upnphttp.o upnpdescgen.o upnpsoap.o \
           upnpreplyparse.o minixml.o \
		   upnpredirect.o getifaddr.o daemonize.o upnpglobalvars.o \
           options.o upnppermissions.o minissdp.o natpmp.o \
           upnpevents.o upnputils.o getconnstatus.o

LNXOBJS = linux/getifstats.o linux/ifacewatcher.o
NETFILTEROBJS = netfilter/iptcrdr.o

ALLOBJS = $(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS)

LIBS += -liptc
# LIBS += -lip4tc
# CFLAGS += -DIPTABLES_143

IPTABLESPATH = ../iptables/
CFLAGS  += -I$(IPTABLESPATH)/include/
LDFLAGS += -L$(IPTABLESPATH)/libiptc/


# test if iptables version >= 1.4.3
##CFLAGS := $(CFLAGS) -DIPTABLES_143
##LIBS = $(IPTABLESPATH)/libiptc/.libs/libip4tc.o
LIBS = $(IPTABLESPATH)/libiptc/libiptc.a

TESTUPNPDESCGENOBJS = testupnpdescgen.o upnpdescgen.o

EXECUTABLES = miniupnpd testupnpdescgen testgetifstats \
              testupnppermissions miniupnpdctl testgetifaddr

.PHONY:	all clean install depend genuuid

all:	$(EXECUTABLES)

clean:
	$(RM) $(ALLOBJS)
	$(RM) $(EXECUTABLES)
	$(RM) testupnpdescgen.o testgetifstats.o
	$(RM) testupnppermissions.o testgetifaddr.o
	$(RM) miniupnpdctl.o

install:	miniupnpd genuuid
	$(STRIP) miniupnpd
	$(INSTALL) -d $(SBININSTALLDIR)
	$(INSTALL) miniupnpd $(SBININSTALLDIR)
	$(INSTALL) -d $(ETCINSTALLDIR)
	$(INSTALL) netfilter/iptables_init.sh $(ETCINSTALLDIR)
	$(INSTALL) netfilter/iptables_removeall.sh $(ETCINSTALLDIR)
	$(INSTALL) --mode=0644 -b miniupnpd.conf $(ETCINSTALLDIR)
	$(INSTALL) -d $(PREFIX)/etc/init.d
	$(INSTALL) linux/miniupnpd.init.d.script $(PREFIX)/etc/init.d/miniupnpd

# genuuid is using the uuidgen CLI tool which is part of libuuid
# from the e2fsprogs
genuuid:
ifeq ($(TARGET_OPENWRT),)
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`(genuuid||uuidgen||uuid) 2>/dev/null`/" miniupnpd.conf
else
	sed -i -e "s/^uuid=[-0-9a-f]*/uuid=`($(STAGING_DIR_HOST)/bin/genuuid||$(STAGING_DIR_HOST)/bin/uuidgen||$(STAGING_DIR_HOST)/bin/uuid) 2>/dev/null`/" miniupnpd.conf
endif

miniupnpd:	$(BASEOBJS) $(LNXOBJS) $(NETFILTEROBJS) $(LIBS)

testupnpdescgen:	$(TESTUPNPDESCGENOBJS)

testgetifstats:	testgetifstats.o linux/getifstats.o

testupnppermissions:	testupnppermissions.o upnppermissions.o

testgetifaddr:	testgetifaddr.o getifaddr.o

miniupnpdctl:	miniupnpdctl.o

config.h:	genconfig.sh
	./genconfig.sh

depend:	config.h
	makedepend -f$(MAKEFILE_LIST) -Y \
	$(ALLOBJS:.o=.c) $(TESTUPNPDESCGENOBJS:.o=.c) \
	testgetifstats.c 2>/dev/null

romfs: miniupnpd
	$(ROMFSINST) miniupnpd /bin/miniupnpd

# DO NOT DELETE

miniupnpd.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
miniupnpd.o: upnphttp.h upnpdescgen.h miniupnpdpath.h getifaddr.h upnpsoap.h
miniupnpd.o: options.h minissdp.h upnpredirect.h daemonize.h upnpevents.h
miniupnpd.o: natpmp.h commonrdr.h upnputils.h ifacewatcher.h
upnphttp.o: config.h upnphttp.h upnpdescgen.h miniupnpdpath.h upnpsoap.h
upnphttp.o: upnpevents.h
upnpdescgen.o: config.h getifaddr.h upnpredirect.h upnpdescgen.h
upnpdescgen.o: miniupnpdpath.h upnpglobalvars.h upnppermissions.h
upnpdescgen.o: miniupnpdtypes.h upnpdescstrings.h upnpurns.h getconnstatus.h
upnpsoap.o: config.h upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
upnpsoap.o: upnphttp.h upnpsoap.h upnpreplyparse.h upnpredirect.h getifaddr.h
upnpsoap.o: getifstats.h getconnstatus.h upnpurns.h
upnpreplyparse.o: upnpreplyparse.h minixml.h
minixml.o: minixml.h
upnpredirect.o: config.h upnpredirect.h upnpglobalvars.h upnppermissions.h
upnpredirect.o: miniupnpdtypes.h upnpevents.h netfilter/iptcrdr.h commonrdr.h
getifaddr.o: config.h getifaddr.h
daemonize.o: daemonize.h config.h
upnpglobalvars.o: config.h upnpglobalvars.h upnppermissions.h
upnpglobalvars.o: miniupnpdtypes.h
options.o: options.h config.h upnppermissions.h upnpglobalvars.h
options.o: miniupnpdtypes.h
upnppermissions.o: config.h upnppermissions.h
minissdp.o: config.h upnpdescstrings.h miniupnpdpath.h upnphttp.h
minissdp.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h minissdp.h
minissdp.o: upnputils.h codelength.h
natpmp.o: config.h natpmp.h upnpglobalvars.h upnppermissions.h
natpmp.o: miniupnpdtypes.h getifaddr.h upnpredirect.h commonrdr.h
upnpevents.o: config.h upnpevents.h miniupnpdpath.h upnpglobalvars.h
upnpevents.o: upnppermissions.h miniupnpdtypes.h upnpdescgen.h
upnputils.o: config.h upnputils.h
getconnstatus.o: getconnstatus.h getifaddr.h
linux/getifstats.o: config.h getifstats.h
linux/ifacewatcher.o: config.h ifacewatcher.h config.h minissdp.h
linux/ifacewatcher.o: miniupnpdtypes.h getifaddr.h upnpglobalvars.h
linux/ifacewatcher.o: upnppermissions.h natpmp.h
netfilter/iptcrdr.o: netfilter/iptcrdr.h commonrdr.h config.h
netfilter/iptcrdr.o: upnpglobalvars.h upnppermissions.h miniupnpdtypes.h
testupnpdescgen.o: config.h upnpdescgen.h
upnpdescgen.o: config.h getifaddr.h upnpredirect.h upnpdescgen.h
upnpdescgen.o: miniupnpdpath.h upnpglobalvars.h upnppermissions.h
upnpdescgen.o: miniupnpdtypes.h upnpdescstrings.h upnpurns.h getconnstatus.h
testgetifstats.o: getifstats.h
