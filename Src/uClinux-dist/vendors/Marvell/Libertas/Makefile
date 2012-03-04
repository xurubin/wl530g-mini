#
#	Makefile -- Build instructions for Marvell/Libertas
#

ROMFSIMG = $(IMAGEDIR)/romfs.img
IMAGE    = $(IMAGEDIR)/image.bin
ELFIMAGE = $(IMAGEDIR)/image.elf

ROMFS_DIRS = bin dev etc home lib/modules mnt proc usr var etc/config etc/dhcpc

ifdef CONFIG_128MHZ_CLOCK
CLOCK=128MHZ
endif

ifdef CONFIG_160MHZ_CLOCK
CLOCK=160MHZ
endif

ifdef CONFIG_8M_MEM
MEMSIZE=8M
endif

ifdef CONFIG_16M_MEM
MEMSIZE=16M
endif

ifndef CLOCK
CLOCK=160MHZ
endif
ifndef MEMSIZE
MEMSIZE=8M
endif

DEVICES = \
	tty,c,5,0      console,c,5,1      cua0,c,5,64      cua1,c,5,65  \
	\
	mem,c,1,1      kmem,c,1,2         null,c,1,3       ram0,b,1,0 \
	ram1,b,1,1 \
	\
	ptyp0,c,2,0    ptyp1,c,2,1        ptyp2,c,2,2      ptyp3,c,2,3 \
	ptyp4,c,2,4    ptyp5,c,2,5        ptyp6,c,2,6      ptyp7,c,2,7 \
	ptyp8,c,2,8    ptyp9,c,2,9        ptypa,c,2,10     ptypb,c,2,11 \
	ptypc,c,2,12   ptypd,c,2,13       ptype,c,2,14     ptypf,c,2,15 \
	\
	rom0,b,31,0    rom1,b,31,1        rom2,b,31,2      rom3,b,31,3 \
	rom4,b,31,4    rom5,b,31,5        rom6,b,31,6      rom7,b,31,7 \
	rom8,b,31,8    rom9,b,31,9 \
	\
	mtd0,c,90,0    mtd1,c,90,2        mtd2,c,90,4      mtd3,c,90,6 \
	mtd4,c,90,8    mtd5,c,90,10 \
	\
	mtdr0,c,90,1   mtdr1,c,90,3       mtdr2,c,90,5     mtdr3,c,90,7 \
	mtdr4,c,90,9   mtdr5,c,90,11 \
	\
	mtdblock0,b,31,0         mtdblock1,b,31,1        mtdblock2,b,31,2 \
	mtdblock3,b,31,3         mtdblock4,b,31,4        mtdblock5,b,31,5 \
	\
	tty0,c,4,0     tty1,c,4,1         tty2,c,4,2       tty3,c,4,3 \
	ttyS0,c,4,64   ttyS1,c,4,65 \
	\
	ttyp0,c,3,0    ttyp1,c,3,1        ttyp2,c,3,2      ttyp3,c,3,3 \
	ttyp4,c,3,4    ttyp5,c,3,5        ttyp6,c,3,6      ttyp7,c,3,7 \
	ttyp8,c,3,8    ttyp9,c,3,9        ttypa,c,3,10     ttypb,c,3,11 \
	ttypc,c,3,12   ttypd,c,3,13       ttype,c,3,14     ttypf,c,3,15 \
	\
	zero,c,1,5     random,c,1,8       urandom,c,1,9


clean:

romfs:
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)

	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	for i in $(DEVICES); do \
		echo $(ROMFSDIR)/dev/$$i | sed 's/,/\t/g' | xargs mknod; \
	done
	$(ROMFSINST) -s /var/tmp /tmp
	$(ROMFSINST) -s /bin /sbin
	$(ROMFSINST) /1
	$(ROMFSINST) /etc/rc
	$(ROMFSINST) /etc/rc.net
	$(ROMFSINST) /etc/inittab
	$(ROMFSINST) /etc/services
	case "$(LINUXDIR)" in \
	*2.4.*) ;; \
	*) echo "ttyS0:linux:/bin/sh" >> $(ROMFSDIR)/etc/inittab ;; \
	esac
	$(ROMFSINST) /etc/inetd.conf
	$(ROMFSINST) /etc/config/dhcpd.conf
	$(ROMFSINST) /etc/config/dhcpd.leases
	$(ROMFSINST) /etc/motd
	$(ROMFSINST) /etc/resolv.conf
	$(ROMFSINST) /etc/resolv.dnsmasq
	$(ROMFSINST) /etc/config/unimac.conf
	$(ROMFSINST) /etc/hostapd.conf
	$(ROMFSINST) /etc/hostapd.accept
	$(ROMFSINST) /etc/hostapd.deny
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	./mkfs.jffs2 -r $(ROOTDIR)/romfs/etc -o jffs2.img
	$(ROMFSINST) /etc/jffs2.img
	$(ROMFSINST) mvwlan-$(MEMSIZE)-$(CLOCK).o /lib/modules/mvwlan.o
	$(ROMFSINST) /etc/updateapd
	$(ROMFSINST) /bin/mvmfgd
	$(ROMFSINST) /bin/mvapd

image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.romvec \
			--remove-section=.text --remove-section=.ramvec \
			--remove-section=.init \
			--remove-section=.bss --remove-section=.eram \
			$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/linux.data
	$(CROSS_COMPILE)objcopy -O binary --remove-section=.ramvec \
			--remove-section=.bss --remove-section=.data \
			--remove-section=.eram \
			--set-section-flags=.romvec=CONTENTS,ALLOC,LOAD,READONLY,CODE \
			$(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/linux.text
	cat $(IMAGEDIR)/linux.text $(IMAGEDIR)/linux.data $(ROMFSIMG) > $(IMAGE)
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROOTDIR)/$(LINUXDIR)/arch/armnommu/boot/zImage /tftpboot
	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(ROMFSIMG) /tftpboot
#	[ -n "$(NO_BUILD_INTO_TFTPBOOT)" ] || cp $(IMAGE) /tftpboot

