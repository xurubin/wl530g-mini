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

.PHONY:		all clean install

MANSRC.1=	l2tpconfig.1
MANSRC.4=	openl2tp_rpc.4
MANSRC.7=	openl2tp.7
MANSRC.8=	openl2tpd.8

MANSRC=		$(MANSRC.1) $(MANSRC.4) $(MANSRC.7) $(MANSRC.8) 

all:		$(MANSRC:%=%.gz)

%.gz: %
		gzip -c $< > $@

clean:
		-$(RM) $(MANSRC:%=%.gz)

install:	all
		$(INSTALL) -d $(DESTDIR)/usr/share/man/man1
		$(INSTALL) $(MANSRC.1:%=%.gz) $(DESTDIR)/usr/share/man/man1
		$(INSTALL) -d $(DESTDIR)/usr/share/man/man4
		$(INSTALL) $(MANSRC.4:%=%.gz) $(DESTDIR)/usr/share/man/man4
		$(INSTALL) -d $(DESTDIR)/usr/share/man/man7
		$(INSTALL) $(MANSRC.7:%=%.gz) $(DESTDIR)/usr/share/man/man7
		$(INSTALL) -d $(DESTDIR)/usr/share/man/man8
		$(INSTALL) $(MANSRC.8:%=%.gz) $(DESTDIR)/usr/share/man/man8


