EXEC =  mvapd
OBJS =  hostapd.o eloop.o ieee802_1x.o eapol_sm.o radius.o md5.o rc4.o \
	common.o iapp.o ieee802_11.o config.o ieee802_11_auth.o accounting.o \
	sta_info.o driver.o receive.o radius_client.o

CFLAGS += -DAP_SW -DBOOSTER_MODE -DTURBO_SETUP -DAP_WPA2 -I../mvwlan -I../wlandrv/incl -g -O2

ifndef CONFIG_LIBERTAS_AP32
CFLAGS += -DPEREGRINE		\
		  -DAUTO_DETECT_MFG
endif

EXTRALIBS = -L./ -lmvapd -L../wlandrv -lwlan 

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $(EXEC) $(OBJS) $(EXTRALIBS) $(LDLIBS) 
	flthdr -s 8192 $(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.elf2flt *.gdb *.o

romfs:
	$(ROMFSINST) /bin/$(EXEC)

convert:
	-find -name '*.[ch]' | xargs $(TOOLSDIR)/dos2unix -k