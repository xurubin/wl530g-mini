OBJS = cache.o rfc1035.o util.o option.o forward.o network.o \
       dnsmasq.o dhcp.o lease.o rfc2131.o netlink.o dbus.o bpf.o \
       helper.o tftp.o log.o conntrack.o

CFLAGS += -Os -DNO_IPV6=1 -DNO_LARGEFILE=1 -DNO_TFTP=1 -DNO_SCRIPT=1
##LIBS += -lc
# -D__USE_BSD=1 
# -DDEFAULT_CACHE_SIZE=20

all : dnsmasq

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

dnsmasq : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 
	
romfs:
	$(ROMFSINST) /bin/dnsmasq

clean :
	rm -f *.o dnsmasq dnsmasq.gdb

.PHONY : all clean romfs