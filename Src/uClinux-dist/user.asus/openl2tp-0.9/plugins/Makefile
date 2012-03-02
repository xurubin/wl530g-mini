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

CFLAGS	:= -DWX54G -Wall -s -O2 -I. -isystem $(KERNEL_SRCDIR)/include -I.. -I../usl $(EXTRA_CFLAGS)
#LDFLAGS	:= -shared

#PLUGINS.c:=	ppp_unix.c ppp_null.c ppp_test.c
PLUGINS.c:=	ppp_unix.c

#PLUGINS.so=	$(PLUGINS.c:%.c=%.so)
PLUGINS.o=	$(PLUGINS.c:%.c=%.o)

SRC.generated=

.PHONY:		all clean install TAGS

#all:		generated-files $(PLUGINS.so)
all:		generated-files $(PLUGINS.o)

clean:
	$(RM) $(PLUGINS.so) $(PLUGINS.o) $(wildcard *.o) $(wildcard *.d) $(SRC.generated)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $<

%.so: %.c
	$(CC) -o $@ $(LDFLAGS) $(CFLAGS) $<

.PHONY:		generated-files

install:		all

generated-files:	$(SRC.generated)

include $(wildcard *.d) /dev/null
