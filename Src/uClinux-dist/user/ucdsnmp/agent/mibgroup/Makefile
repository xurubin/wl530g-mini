# Generated automatically from Makefile.in by configure.
#
# Minimum environment and virtual path setup
#
SHELL		= /bin/sh
srcdir		= .
top_srcdir	= ../..


#
# Paths
#
prefix		= /usr/local
exec_prefix	= /usr/local
bindir		= ${exec_prefix}/bin
sbindir		= ${exec_prefix}/sbin
libdir		= ${exec_prefix}/lib
datadir		= ${prefix}/share
includedir	= ${prefix}/include/ucd-snmp
mandir		= ${prefix}/man
man1dir		= $(mandir)/man1
man3dir		= $(mandir)/man3
man5dir		= $(mandir)/man5
man8dir		= $(mandir)/man8
snmplibdir	= $(datadir)/snmp
mibdir		= $(snmplibdir)/mibs
persistentdir	= /tmp


#
# Programs
#
INSTALL		= /usr/bin/install -c
SED		= sed
#RANLIB		= ranlib
LN_S		= ln -s

#
# Compiler arguments
#
# CFLAGS		= -g -O2 -Dlinux
EXTRACPPFLAGS	= -x c
# LDFLAGS		=  

#
# Shared library commands (or not)
#
SHLIB_CFLAGS		= 
SHLIB_EXTENSION		= a
SHLIB_VERSION		= 
SHLIB_LDCONFIG_CMD	= :
SHLIB_LD_CMD		= $(AR) cr
SHLIB_LD_LIBS		= 
CC_RUNTIME_ARG		= 


CPPFLAGS=-I. -I../.. -I$(srcdir) -I$(srcdir)/../.. \
	-I$(srcdir)/../../snmplib -I$(srcdir)/.. -I..  $(SHLIB_CFLAGS) $(INCS)
# CC = gcc

# Need a special .c -> .o definition here to make sure we place the
# object files in the sub directory.

.c.o:
	$(CC) -c $(CPPFLAGS) $(CFLAGS) -o $@ $<

OBJS =  misc/ipfwacc.o mibII/system_mib.o mibII/sysORTable.o mibII/at.o mibII/interfaces.o mibII/snmp_mib.o mibII/tcp.o mibII/icmp.o mibII/ip.o mibII/udp.o mibII/vacm_vars.o ucd-snmp/memory.o ucd-snmp/proc.o ucd-snmp/versioninfo.o ucd-snmp/pass.o ucd-snmp/pass_persist.o ucd-snmp/disk.o ucd-snmp/loadave.o ucd-snmp/extensible.o ucd-snmp/errormib.o ucd-snmp/registry.o ucd-snmp/file.o snmpv3/snmpEngine.o snmpv3/snmpMPDStats.o snmpv3/usmStats.o snmpv3/usmUser.o util_funcs.o mibII/var_route.o mibII/route_write.o 
SRCS =  misc/ipfwacc.c mibII/system_mib.c mibII/sysORTable.c mibII/at.c mibII/interfaces.c mibII/snmp_mib.c mibII/tcp.c mibII/icmp.c mibII/ip.c mibII/udp.c mibII/vacm_vars.c ucd-snmp/memory.c ucd-snmp/proc.c ucd-snmp/versioninfo.c ucd-snmp/pass.c ucd-snmp/pass_persist.c ucd-snmp/disk.c ucd-snmp/loadave.c ucd-snmp/extensible.c ucd-snmp/errormib.c ucd-snmp/registry.c ucd-snmp/file.c snmpv3/snmpEngine.c snmpv3/snmpMPDStats.c snmpv3/usmStats.c snmpv3/usmUser.c util_funcs.c mibII/var_route.c mibII/route_write.c 
# OBJS =  mibII/system_mib.o mibII/sysORTable.o mibII/at.o mibII/interfaces.o mibII/snmp_mib.o mibII/tcp.o mibII/icmp.o mibII/ip.o mibII/udp.o mibII/vacm_vars.o ucd-snmp/memory.o ucd-snmp/vmstat.o ucd-snmp/proc.o ucd-snmp/versioninfo.o ucd-snmp/pass.o ucd-snmp/pass_persist.o ucd-snmp/disk.o ucd-snmp/loadave.o ucd-snmp/extensible.o ucd-snmp/errormib.o ucd-snmp/registry.o ucd-snmp/file.o snmpv3/snmpEngine.o snmpv3/snmpMPDStats.o snmpv3/usmStats.o snmpv3/usmUser.o util_funcs.o mibII/var_route.o mibII/route_write.o 
# SRCS =  mibII/system_mib.c mibII/sysORTable.c mibII/at.c mibII/interfaces.c mibII/snmp_mib.c mibII/tcp.c mibII/icmp.c mibII/ip.c mibII/udp.c mibII/vacm_vars.c ucd-snmp/memory.c ucd-snmp/vmstat.c ucd-snmp/proc.c ucd-snmp/versioninfo.c ucd-snmp/pass.c ucd-snmp/pass_persist.c ucd-snmp/disk.c ucd-snmp/loadave.c ucd-snmp/extensible.c ucd-snmp/errormib.c ucd-snmp/registry.c ucd-snmp/file.c snmpv3/snmpEngine.c snmpv3/snmpMPDStats.c snmpv3/usmStats.c snmpv3/usmUser.c util_funcs.c mibII/var_route.c mibII/route_write.c 

all: $(OBJS)

TAGS: $(SRCS)
	etags *.c *.h

clean:
	rm -f ${OBJS}

depend: 
	makedepend $(CPPFLAGS) $(SRCS)

nosysdepend:
	makedepend $(CPPFLAGS) $(SRCS)
	perl -n -i.bak $(top_srcdir)/makenosysdepend.pl Makefile

makefileindepend: nosysdepend
	perl $(top_srcdir)/makefileindepend.pl

# DO NOT DELETE THIS LINE -- make depend depends on it.

mibII/at.o: mibII/at.h mibII/interfaces.h mibincl.h
mibII/icmp.o: mibII/icmp.h mibII/sysORTable.h mibincl.h
mibII/interfaces.o: mibII/interfaces.h mibII/sysORTable.h mibincl.h
mibII/ip.o: mibII/at.h mibII/interfaces.h mibII/ip.h mibII/route_write.h
mibII/ip.o: mibII/sysORTable.h mibII/var_route.h mibincl.h
mibII/route_write.o: mibII/at.h mibII/ip.h mibII/route_write.h
mibII/route_write.o: mibII/var_route.h
mibII/snmp_mib.o: mibII/snmp_mib.h mibII/sysORTable.h mibincl.h struct.h
mibII/snmp_mib.o: util_funcs.h
mibII/sysORTable.o: mibII/sysORTable.h mibincl.h
mibII/system_mib.o: mibII/sysORTable.h mibII/system_mib.h mibincl.h
mibII/tcp.o: mibII/sysORTable.h mibII/tcp.h mibincl.h
mibII/udp.o: mibII/sysORTable.h mibII/tcp.h mibII/udp.h mibincl.h
mibII/vacm_vars.o: mibII/sysORTable.h mibII/vacm_vars.h mibincl.h
mibII/var_route.o: mibII/at.h mibII/interfaces.h mibII/ip.h
mibII/var_route.o: mibII/route_write.h mibII/var_route.h struct.h
mibII/var_route.o: util_funcs.h
misc/ipfwacc.o: mibincl.h misc/ipfwacc.h struct.h
misc/ipfwacc.o: util_funcs.h
snmpv3/snmpEngine.o: mibincl.h snmpv3/snmpEngine.h struct.h
snmpv3/snmpEngine.o: util_funcs.h
snmpv3/snmpMPDStats.o: mibincl.h snmpv3/snmpMPDStats.h struct.h
snmpv3/snmpMPDStats.o: util_funcs.h
snmpv3/usmStats.o: mibincl.h snmpv3/usmStats.h struct.h
snmpv3/usmStats.o: util_funcs.h
snmpv3/usmUser.o: mibincl.h snmpv3/usmUser.h struct.h
snmpv3/usmUser.o: util_funcs.h
ucd-snmp/disk.o: mibdefs.h mibincl.h struct.h ucd-snmp/disk.h
ucd-snmp/disk.o: ucd-snmp/errormib.h
ucd-snmp/disk.o: util_funcs.h
ucd-snmp/errormib.o: mibdefs.h mibincl.h struct.h ucd-snmp/errormib.h
ucd-snmp/errormib.o: util_funcs.h
ucd-snmp/extensible.o: mibdefs.h mibincl.h struct.h ucd-snmp/extensible.h
ucd-snmp/extensible.o: util_funcs.h
ucd-snmp/file.o: mibdefs.h mibincl.h struct.h ucd-snmp/file.h
ucd-snmp/file.o: util_funcs.h
ucd-snmp/loadave.o: mibdefs.h mibincl.h struct.h ucd-snmp/loadave.h
ucd-snmp/loadave.o: util_funcs.h
ucd-snmp/pass.o: mibdefs.h mibincl.h struct.h ucd-snmp/extensible.h
ucd-snmp/pass.o: ucd-snmp/pass.h
ucd-snmp/pass.o: util_funcs.h
ucd-snmp/pass_persist.o: mibdefs.h mibincl.h struct.h ucd-snmp/extensible.h
ucd-snmp/pass_persist.o: ucd-snmp/pass_persist.h
ucd-snmp/pass_persist.o: util_funcs.h
ucd-snmp/proc.o: mibdefs.h mibincl.h struct.h ucd-snmp/errormib.h
ucd-snmp/proc.o: ucd-snmp/proc.h
ucd-snmp/proc.o: util_funcs.h
ucd-snmp/registry.o: mibincl.h ucd-snmp/registry.h
ucd-snmp/versioninfo.o: mibdefs.h mibincl.h struct.h ucd-snmp/versioninfo.h
ucd-snmp/versioninfo.o: util_funcs.h
util_funcs.o: ../agent_read_config.h ../agent_registry.h ../agent_trap.h
util_funcs.o: ../../config.h ../../m/generic.h mibdefs.h mibincl.h
util_funcs.o: ../../mib_module_config.h ../../s/generic.h ../../s/linux.h
util_funcs.o: ../../snmplib/asn1.h ../../snmplib/callback.h
util_funcs.o: ../../snmplib/getopt.h ../../snmplib/mib.h
util_funcs.o: ../../snmplib/read_config.h ../../snmplib/snmp_api.h
util_funcs.o: ../../snmplib/snmp_debug.h ../../snmplib/snmp.h
util_funcs.o: ../../snmplib/snmp_impl.h ../../snmplib/snmp_logging.h
util_funcs.o: ../../snmplib/system.h ../../snmplib/tools.h ../snmp_vars.h
util_funcs.o: ../../s/sysv.h struct.h ucd-snmp/errormib.h
util_funcs.o: util_funcs.h ../var_struct.h
