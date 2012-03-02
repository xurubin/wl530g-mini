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

CFLAGS	:= -s -MMD -Wall -O2 -isystem ../kernel/include -I. -I.. -I.. -I../usl -fPIC $(EXTRA_CFLAGS)
#eric--LDFLAGS	:= -shared
LDFLAGS	:= -static -Wl,-elf2flt="-s 8000"

PLUGINS.c:=	ppp_unix.c ppp_null.c ppp_test.c

PLUGINS.so=	$(PLUGINS.c:%.c=%.so)
PLUGINS.o=	$(PLUGINS.c:%.c=%.o)

SRC.generated=

.PHONY:		all clean install TAGS

#eric--all:		generated-files $(PLUGINS.so)
all:		generated-files $(PLUGINS.o)

clean:
	$(RM) $(PLUGINS.so) $(wildcard *.o) $(wildcard *.d) $(SRC.generated) $(PLUGINS.o)

%.so: %.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $<

#eric++
%.o: %.c
	$(CC) -c -o $@ $(LDFLAGS) $(CFLAGS) $<

.PHONY:		generated-files

install:		all
			$(INSTALL) -d $(DESTDIR)/usr/lib/openl2tp
			$(INSTALL) $(PLUGINS.so) $(DESTDIR)/usr/lib/openl2tp


generated-files:	$(SRC.generated)

include $(wildcard *.d) /dev/null
