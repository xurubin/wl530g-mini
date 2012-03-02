
EXEC = rgcfg
OBJS = rgcfg.o

CFLAGS += -I../ffnetdrv/include -I../ffnetdrv/qd_drv/include -I../ffnetdrv/unimac
all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS$(LDLIBS-$(@)))

romfs:
	$(ROMFSINST) /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o

