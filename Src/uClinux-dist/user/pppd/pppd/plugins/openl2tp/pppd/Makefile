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

# BEGIN CONFIGURABLE SETTINGS

ifdef PPPD_SRCDIR
CFLAGS.ppp=	-I$(PPPD_SRCDIR)/include -I$(PPPD_SRCDIR)/pppd
endif
ifndef PPPD_LIBDIR
PPPD_LIBDIR=	/usr/lib/pppd/$(PPPD_VERSION)
endif

# END CONFIGURABLE SETTINGS

CFLAGS	+= -g -isystem ../kernel/include -I.. -I/usr/include/pppd $(CFLAGS.pppd) -fPIC -I../../..
#LDFLAGS	+= -shared

all:	pppol2tp.a

pppol2tp.a: pppol2tp.o ../l2tp_event_rpc_xdr.o ../l2tp_event_rpc_client.o
	$(AR) -rc $@ $^

pppol2tp.c: pppol2tp.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) -D_linux_=1 $^

install: pppol2tp.a
	$(INSTALL) -d $(DESTDIR)$(PPPD_LIBDIR)
	$(INSTALL) pppol2tp.so $(DESTDIR)$(PPPD_LIBDIR)

clean:
	-rm -rf *.o *.so
