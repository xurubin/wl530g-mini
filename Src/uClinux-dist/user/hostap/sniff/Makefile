ifndef CFLAGS
CFLAGS = -O2 -Wall
endif

OBJS =	wlansniff.o management.o

all: wlansniff

wlansniff: $(OBJS)
	$(CC) -o wlansniff $(OBJS) -DPRISM2_MONITOR

wlansniff.o: wlansniff.c ieee80211.h
management.o: management.c ieee80211.h


clean:
	rm -f core *.o wlansniff
