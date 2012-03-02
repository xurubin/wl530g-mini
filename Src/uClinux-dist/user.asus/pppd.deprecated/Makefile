
.EXPORT_ALL_VARIABLES:

DIRS = pppd

#eric++
CC = arm-elf-gcc
LD = arm-elf-ld
AR = arm-elf-ar
CFLAGS  += -s -O2
LDFLAGS = -Wl,-elf2flt="-s 8000"

.PHONY: all romfs clean install

all:
	make -C pppd/plugins
	for i in $(DIRS) ; do make -C $$i || exit $? ; done

romfs:
	for i in $(DIRS) ; do make -C $$i romfs || exit $? ; done

clean:
	-for i in $(DIRS) ; do make -C $$i clean ; done

install:
	cp -f pppd/pppd /Marvell/Src/uClinux-dist/filesystem/target/bin
	cp -f pppd/plugins/openl2tp/openl2tpd /Marvell/Src/uClinux-dist/filesystem/target/sbin

